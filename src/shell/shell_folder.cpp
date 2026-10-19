// \file shell_folder.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "shell_folder.h"
#include "change_notify_watcher_p.h"
#include "icon_cache_p.h"
#include "shell_folder_p.h"
#include "shell_item_p.h"
#include "shell_pidl_p.h"

#include <ntquery.h>
#include <shlwapi.h>

ShellFolder::ShellFolder(const QString& path, QWidget* parent)
    : QObject(parent)
    , d(new ShellFolderPrivate())
{
    d->q = this;

    auto windowsPathStyle = path;
    windowsPathStyle.replace(QLatin1Char('/'), QLatin1Char('\\'));
    HRESULT hr = SHParseDisplayName((PCWSTR)windowsPathStyle.utf16(), nullptr, &d->m_pidl, 0, nullptr);

    if (SUCCEEDED(hr))
    {
        hr = SHBindToObject(nullptr, d->m_pidl, nullptr, IID_PPV_ARGS(&d->m_folder));

        if (SUCCEEDED(hr)) d->readProperties();
    }
}

ShellFolder::ShellFolder(const ShellPidl& pidl, QWidget* parent)
    : QObject(parent)
    , d(new ShellFolderPrivate())
{
    d->q = this;

    d->m_pidl = ILClone(pidl.d->pidl());

    HRESULT hr = SHBindToObject(nullptr, d->m_pidl, nullptr, IID_PPV_ARGS(&d->m_folder));

    if (SUCCEEDED(hr)) d->readProperties();
}

ShellFolder::ShellFolder(IShellFolder* folder, QWidget* parent)
    : QObject(parent)
    , d(new ShellFolderPrivate())
{
    d->q = this;
    d->m_folder = folder;

    HRESULT hr = SHGetIDListFromObject(folder, &d->m_pidl);

    if (SUCCEEDED(hr)) d->readProperties();
}

ShellFolder::~ShellFolder()
{
    delete d;
}

ShellFolderPrivate::ShellFolderPrivate()
    : q(nullptr)
    , m_hasParent(false)
{
}

ShellFolderPrivate::~ShellFolderPrivate()
{
}

void ShellFolderPrivate::readProperties()
{
    wchar_t* name;
    HRESULT hr = SHGetNameFromIDList(m_pidl, SIGDN_DESKTOPABSOLUTEEDITING, &name);

    if (SUCCEEDED(hr))
    {
        m_path = QString::fromWCharArray(name);
        m_path.replace(QLatin1Char('\\'), QLatin1Char('/'));
        CoTaskMemFree(name);
    }

    if (m_path.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive))
    {
        auto url = QUrl(m_path);
        m_path = url.toString(QUrl::RemoveUserInfo);
        m_path.replace(QLatin1Char('\\'), QLatin1Char('/'));

        m_user = url.userName();
        m_password = url.password();
    }

    m_hasParent = true;

    LPITEMIDLIST pidl;
    hr = SHGetSpecialFolderLocation((HWND)q->parent()->effectiveWinId(), CSIDL_DESKTOP, &pidl);

    if (SUCCEEDED(hr))
    {
        if (ILIsEqual(pidl, m_pidl)) m_hasParent = false;

        CoTaskMemFree(pidl);
    }

    const int rootFolders[3] = {CSIDL_DRIVES, CSIDL_NETWORK, CSIDL_INTERNET};

    for (int i = 0; i < 3 && m_hasParent; i++)
    {
        LPITEMIDLIST pidl;
        hr = SHGetSpecialFolderLocation((HWND)q->parent()->effectiveWinId(), rootFolders[i], &pidl);

        if (SUCCEEDED(hr))
        {
            if (ILIsParent(pidl, m_pidl, true)) m_hasParent = false;

            CoTaskMemFree(pidl);
        }
    }
}

bool ShellFolder::isValid() const
{
    return d->m_folder != nullptr;
}

QString ShellFolder::path() const
{
    return d->m_path;
}

bool ShellFolder::hasParent() const
{
    return d->m_hasParent;
}

ShellItem::Attributes ShellFolder::attributes()
{
    ShellItem::Attributes result;

    ShellItem item;
    ShellFolder* parentFolder = d->createParentFolder(item);

    if (parentFolder)
    {
        parentFolder->d->readItemProperties(item);
        result = item.d->m_attributes;

        delete parentFolder;
    }

    return result;
}

QString ShellFolder::name()
{
    QString result;

    wchar_t* name;
    HRESULT hr = SHGetNameFromIDList(d->m_pidl, SIGDN_PARENTRELATIVEEDITING, &name);

    if (SUCCEEDED(hr))
    {
        result = QString::fromWCharArray(name);
        CoTaskMemFree(name);
    }

    return result;
}

ShellPidl ShellFolder::pidl() const
{
    ShellPidl pidl;
    pidl.d->m_data = QByteArray((const char*)d->m_pidl, (int)ILGetSize(d->m_pidl));
    pidl.d->m_path = d->m_path;
    return pidl;
}

QString ShellFolder::user() const
{
    return d->m_user;
}

QString ShellFolder::password() const
{
    return d->m_password;
}

QList<ShellItem> ShellFolder::listItems(Flags flags)
{
    QList<ShellItem> result;

    SHCONTF enumFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    if (flags.testFlag(IncludeHidden)) enumFlags |= SHCONTF_INCLUDEHIDDEN;

    IEnumIDList* enumerator;
    HRESULT hr = d->m_folder->EnumObjects((HWND)parent()->effectiveWinId(), enumFlags, &enumerator);

    if (SUCCEEDED(hr) && enumerator)
    {
        LPITEMIDLIST pidl;
        while (enumerator->Next(1, &pidl, nullptr) == S_OK)
            result.append(d->makeItem(pidl));

        enumerator->Release();
    }

    return result;
}

ShellItem ShellFolderPrivate::makeItem(LPITEMIDLIST pidl)
{
    ShellItem item;
    item.d->m_pidl = pidl;
    readItemProperties(item);
    return item;
}

ShellItem ShellFolderPrivate::makeNotifyItem(LPITEMIDLIST pidl)
{
    ShellItem item;
    item.d->m_pidl = ILClone(pidl);
    return item;
}

ShellItem ShellFolderPrivate::makeRealNotifyItem(LPITEMIDLIST pidl)
{
    ShellItem item;

    HRESULT hr = SHGetRealIDL(m_folder, pidl, &item.d->m_pidl);

    if (SUCCEEDED(hr)) readItemProperties(item);

    return item;
}

static QDateTime systemTimeToQDateTime(const SYSTEMTIME* systemTime)
{
    SYSTEMTIME localTime;
    SystemTimeToTzSpecificLocalTime(nullptr, systemTime, &localTime);

    QDate date(localTime.wYear, localTime.wMonth, localTime.wDay);
    QTime time(localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);

    return QDateTime(date, time);
}

static QDateTime fileTimeToQDateTime(const FILETIME* fileTime)
{
    SYSTEMTIME systemTime;
    FileTimeToSystemTime(fileTime, &systemTime);

    return systemTimeToQDateTime(&systemTime);
}

static QDateTime variantTimeToQDateTime(double vtime)
{
    SYSTEMTIME systemTime;
    VariantTimeToSystemTime(vtime, &systemTime);

    return systemTimeToQDateTime(&systemTime);
}

static ShellItem::Attributes fileAttributesToAttributes(DWORD fileAttributes)
{
    ShellItem::Attributes attributes;

    if (fileAttributes & FILE_ATTRIBUTE_READONLY) attributes |= ShellItem::ReadOnly;
    if (fileAttributes & FILE_ATTRIBUTE_HIDDEN) attributes |= ShellItem::Hidden;
    if (fileAttributes & FILE_ATTRIBUTE_SYSTEM) attributes |= ShellItem::System;
    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) attributes |= ShellItem::Directory;
    if (fileAttributes & FILE_ATTRIBUTE_ARCHIVE) attributes |= ShellItem::Archive;
    if (fileAttributes & FILE_ATTRIBUTE_COMPRESSED) attributes |= ShellItem::Compressed;
    if (fileAttributes & FILE_ATTRIBUTE_ENCRYPTED) attributes |= ShellItem::Encrypted;
    if (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) attributes |= ShellItem::ReparsePoint;

    return attributes;
}

void ShellFolderPrivate::readItemProperties(ShellItem& item)
{
    item.d->m_name = displayName(item.d->m_pidl, SHGDN_INFOLDER);

    ShellItem::Attributes attributes;
    ShellItem::State state;

    ULONG shellAttributes = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_FILESYSTEM | SFGAO_CANRENAME;
    HRESULT hr = m_folder->GetAttributesOf(1, (LPCITEMIDLIST*)&item.d->m_pidl, &shellAttributes);

    if (SUCCEEDED(hr))
    {
        if (shellAttributes & SFGAO_FOLDER) attributes |= ShellItem::Folder;
        if (shellAttributes & SFGAO_STREAM) attributes |= ShellItem::Stream;
        if (shellAttributes & SFGAO_FILESYSTEM) attributes |= ShellItem::FileSystem;
        if (shellAttributes & SFGAO_CANRENAME) attributes |= ShellItem::CanRename;
    }

    WIN32_FIND_DATA data;
    hr = SHGetDataFromIDList(m_folder, item.d->m_pidl, SHGDFIL_FINDDATA, &data, sizeof(data));

    if (SUCCEEDED(hr))
    {
        attributes |= fileAttributesToAttributes(data.dwFileAttributes);

        item.d->m_name = QString::fromWCharArray(data.cFileName);

        item.d->m_size = (quint64)data.nFileSizeHigh << 32 | data.nFileSizeLow;

        item.d->m_modified = fileTimeToQDateTime(&data.ftLastWriteTime);

        state |= ShellItem::HasProperties;
    }
    else
    {
        if (attributes.testFlag(ShellItem::Folder) && !attributes.testFlag(ShellItem::Stream)) attributes |= ShellItem::Directory;

        IShellFolder2* folder2;
        hr = m_folder->QueryInterface(IID_PPV_ARGS(&folder2));

        if (SUCCEEDED(hr))
        {
            SHCOLUMNID shcolumnid;
            shcolumnid.fmtid = FMTID_Storage;
            shcolumnid.pid = PID_STG_WRITETIME;

            VARIANT variant = {0};

            hr = folder2->GetDetailsEx(item.d->m_pidl, &shcolumnid, &variant);

            if (SUCCEEDED(hr))
            {
                item.d->m_modified = variantTimeToQDateTime(variant.date);

                shcolumnid.pid = PID_STG_ATTRIBUTES;
                hr = folder2->GetDetailsEx(item.d->m_pidl, &shcolumnid, &variant);

                if (SUCCEEDED(hr)) attributes |= fileAttributesToAttributes(variant.uintVal);

                if (!attributes.testFlag(ShellItem::Directory))
                {
                    shcolumnid.pid = PID_STG_SIZE;
                    hr = folder2->GetDetailsEx(item.d->m_pidl, &shcolumnid, &variant);

                    if (SUCCEEDED(hr)) item.d->m_size = variant.llVal;
                }

                state |= ShellItem::HasProperties;
            }

            folder2->Release();
        }
    }

    item.d->m_attributes = attributes;
    item.d->m_state = state;

    QPixmap icon;

    if (attributes.testFlag(ShellItem::Directory))
    {
        icon = IconCache::directoryIcon();
    }
    else
    {
        QString extension;
        int pos = item.d->m_name.lastIndexOf('.');
        if (pos > 0) extension = item.d->m_name.mid(pos + 1).toLower();

        if (!extension.isEmpty()) icon = IconCache::loadFileIcon(extension);

        if (icon.isNull()) icon = IconCache::fileIcon();
    }

    item.d->m_icon = icon;
}

bool ShellFolder::extractIcon(ShellItem& item)
{
    bool result = false;

    QPixmap icon = d->extractIcon(item.d->m_pidl);

    if (!icon.isNull())
    {
        item.d->m_icon = icon;
        result = true;
    }

    item.d->m_state |= ShellItem::HasExtractedIcon;

    return result;
}

qint64 ShellFolderPrivate::calculateSize(IShellFolder* parentFolder, LPITEMIDLIST pidl)
{
    qint64 size = 0;

    IShellFolder* folder;
    HRESULT hr = SHBindToObject(parentFolder, pidl, nullptr, IID_PPV_ARGS(&folder));

    if (SUCCEEDED(hr))
    {
        IEnumIDList* enumerator;
        hr = folder->EnumObjects((HWND)q->parent()->effectiveWinId(), SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &enumerator);

        if (SUCCEEDED(hr) && enumerator)
        {
            LPITEMIDLIST itemPidl;
            while (enumerator->Next(1, &itemPidl, nullptr) == S_OK)
            {
                WIN32_FIND_DATA data;
                hr = SHGetDataFromIDList(folder, itemPidl, SHGDFIL_FINDDATA, &data, sizeof(data));

                if (SUCCEEDED(hr))
                {
                    size += (qint64)data.nFileSizeHigh << 32 | data.nFileSizeLow;

                    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                        size += calculateSize(folder, itemPidl);
                }

                CoTaskMemFree(itemPidl);
            }

            enumerator->Release();
        }

        folder->Release();
    }

    return size;
}

bool ShellFolder::calculateSize(ShellItem& item)
{
    if (item.d->m_state.testFlag(ShellItem::HasCalculatedSize) || !item.d->m_state.testFlag(ShellItem::HasProperties)) return false;

    if (!item.d->m_attributes.testFlag(ShellItem::Directory)) return false;

    item.d->m_size = d->calculateSize(d->m_folder, item.d->m_pidl);
    item.d->m_state |= ShellItem::HasCalculatedSize;

    return true;
}

bool ShellFolder::setItemName(ShellItem& item, const QString& name)
{
    bool result = false;

    LPITEMIDLIST pidlNew = nullptr;
    HRESULT hr = d->m_folder->SetNameOf((HWND)parent()->effectiveWinId(), item.d->m_pidl, (LPCWSTR)name.utf16(), SHGDN_INFOLDER | SHGDN_FORPARSING, &pidlNew);

    if (SUCCEEDED(hr))
    {
        if (pidlNew)
        {
            LPITEMIDLIST absolutePidl = ILCombine(d->m_pidl, item.d->m_pidl);
            LPITEMIDLIST absolutePidlNew = ILCombine(d->m_pidl, pidlNew);

            SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, absolutePidl, absolutePidlNew);

            CoTaskMemFree(absolutePidl);
            CoTaskMemFree(absolutePidlNew);

            CoTaskMemFree(item.d->m_pidl);
            item.d->m_pidl = pidlNew;
        }

        item.d->m_name = name;
        item.d->m_state &= ~ShellItem::HasExtractedIcon;

        result = true;
    }

    return result;
}

QString ShellFolder::itemPath(const ShellItem& item) const
{
    QString result;

    LPITEMIDLIST absolutePidl = ILCombine(d->m_pidl, item.d->m_pidl);

    wchar_t buffer[1024];
    if (SHGetPathFromIDListEx(absolutePidl, buffer, 1024, GPFIDL_DEFAULT)) result = QString::fromWCharArray(buffer);

    CoTaskMemFree(absolutePidl);

    return result;
}

ShellPidl ShellFolder::itemPidl(const ShellItem& item) const
{
    ShellPidl pidl;

    LPITEMIDLIST absolutePidl = ILCombine(d->m_pidl, item.d->m_pidl);

    pidl.d->m_data = QByteArray((const char*)absolutePidl, (int)ILGetSize(absolutePidl));

    wchar_t buffer[1024];
    if (SHGetPathFromIDListEx(absolutePidl, buffer, 1024, GPFIDL_DEFAULT)) pidl.d->m_path = QString::fromWCharArray(buffer);

    pidl.d->m_attributes = item.d->m_attributes;

    CoTaskMemFree(absolutePidl);

    return pidl;
}

bool ShellFolder::isEqual(const ShellFolder* other) const
{
    return ILIsEqual(d->m_pidl, other->d->m_pidl);
}

ShellFolder* ShellFolder::clone()
{
    d->m_folder->AddRef();
    return new ShellFolder(d->m_folder, parent());
}

ShellFolder* ShellFolder::openFolder(const ShellItem& item)
{
    if (!item.attributes().testFlag(ShellItem::Folder)) return nullptr;

    ShellFolder* result = nullptr;

    IShellFolder* folder;
    HRESULT hr = SHBindToObject(d->m_folder, item.d->m_pidl, nullptr, IID_PPV_ARGS(&folder));

    if (SUCCEEDED(hr)) result = new ShellFolder(folder, parent());

    return result;
}

ShellFolder* ShellFolder::parentFolder(ShellItem& item)
{
    if (!hasParent()) return nullptr;

    return d->createParentFolder(item);
}

ShellFolder* ShellFolderPrivate::createParentFolder(ShellItem& item)
{
    ShellFolder* result = nullptr;

    IShellFolder* folder;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToParent(m_pidl, IID_PPV_ARGS(&folder), &pidlLast);

    if (SUCCEEDED(hr))
    {
        item.d->m_pidl = ILClone(pidlLast);

        result = new ShellFolder(folder, q->parent());
    }

    return result;
}

ShellFolder* ShellFolder::rootFolder()
{
    ShellFolder* result = nullptr;

    for (const ShellFolder* folder = this; folder && folder->hasParent(); folder = result)
    {
        ShellItem item;
        result = folder->d->createParentFolder(item);

        if (folder != this) delete folder;
    }

    return result;
}

static int CALLBACK ShellFolderBrowseCallbackProc(HWND hwnd, UINT msg, LPARAM /*lparam*/, LPARAM data)
{
    if (msg == BFFM_INITIALIZED) SendMessage(hwnd, BFFM_SETSELECTION, false, data);
    return 0;
}

ShellFolder* ShellFolder::browseFolder()
{
    ShellFolder* result = nullptr;

    wchar_t buffer[MAX_PATH];

    QString title = tr("Select the folder to open:");

    BROWSEINFO info = {0};
    info.hwndOwner = (HWND)parent()->effectiveWinId();
    info.pidlRoot = nullptr;
    info.pszDisplayName = buffer;
    info.lpszTitle = (LPCWSTR)title.utf16();
    info.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    info.lpfn = ShellFolderBrowseCallbackProc;
    info.lParam = (LPARAM)d->m_pidl;

    LPITEMIDLIST pidl = SHBrowseForFolder(&info);

    if (pidl)
    {
        IShellFolder* folder;
        HRESULT hr = SHBindToObject(nullptr, pidl, nullptr, IID_PPV_ARGS(&folder));

        if (SUCCEEDED(hr)) result = new ShellFolder(folder, parent());

        CoTaskMemFree(pidl);
    }

    return result;
}

ShellPidl ShellFolder::browseFolder(QWidget* parent, const QString& title, const ShellPidl& startPidl)
{
    ShellPidl result;

    wchar_t buffer[MAX_PATH];

    BROWSEINFO info = {0};
    info.hwndOwner = (HWND)parent->effectiveWinId();
    info.pidlRoot = nullptr;
    info.pszDisplayName = buffer;
    info.lpszTitle = (LPCWSTR)title.utf16();
    info.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    info.lpfn = ShellFolderBrowseCallbackProc;
    info.lParam = (LPARAM)startPidl.d->pidl();

    LPITEMIDLIST pidl = SHBrowseForFolder(&info);

    if (pidl)
    {
        result.d->m_data = QByteArray((const char*)pidl, (int)ILGetSize(pidl));

        wchar_t realPath[1024];
        if (SHGetPathFromIDListEx(pidl, realPath, 1024, GPFIDL_DEFAULT))
            result.d->m_path = QString::fromWCharArray(realPath);
        else
            result.d->m_path = QString::fromWCharArray(buffer);

        CoTaskMemFree(pidl);
    }

    return result;
}

ShellPidl ShellFolder::defaultFolder()
{
    ShellPidl result;

    result.d->m_path = QDir::toNativeSeparators(QDir::rootPath());

    LPITEMIDLIST pidl;
    HRESULT hr = SHParseDisplayName((PCWSTR)result.d->m_path.utf16(), nullptr, &pidl, 0, nullptr);

    if (SUCCEEDED(hr))
    {
        result.d->m_data = QByteArray((const char*)pidl, (int)ILGetSize(pidl));

        CoTaskMemFree(pidl);
    }

    return result;
}

bool ShellFolder::canCreateFolder()
{
    bool result = false;

    ITransferDestination* destination;
    HRESULT hr = d->m_folder->CreateViewObject((HWND)parent()->effectiveWinId(), IID_PPV_ARGS(&destination));

    if (SUCCEEDED(hr))
    {
        destination->Release();
        result = true;
    }

    if (!result)
    {
        ShellItem::Attributes attrs = attributes();
        if ((attrs & ShellItem::FileSystem) && (attrs & ShellItem::Directory)) result = true;
    }

    return result;
}

bool ShellFolder::createFolder(const QString& name)
{
    bool result = false;

    IFileOperation* fileOperation;
    HRESULT hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileOperation));

    if (SUCCEEDED(hr))
    {
        fileOperation->SetOwnerWindow((HWND)parent()->effectiveWinId());

        DWORD operationFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOFX_NOMINIMIZEBOX;
        fileOperation->SetOperationFlags(operationFlags);

        IShellItem* item;
        hr = SHCreateItemFromIDList(d->m_pidl, IID_PPV_ARGS(&item));

        if (SUCCEEDED(hr))
        {
            hr = fileOperation->NewItem(item, FILE_ATTRIBUTE_DIRECTORY, (LPCWSTR)name.utf16(), nullptr, nullptr);

            if (SUCCEEDED(hr))
            {
                hr = fileOperation->PerformOperations();

                if (SUCCEEDED(hr)) result = true;
            }

            item->Release();
        }

        fileOperation->Release();
    }

    if (!result)
    {
        ShellItem::Attributes attrs = attributes();
        if ((attrs & ShellItem::FileSystem) && (attrs & ShellItem::Directory))
        {
            QString path = d->m_path + QLatin1Char('/') + name;

            result = CreateDirectory((LPCWSTR)path.utf16(), nullptr);
        }
    }

    return result;
}

bool ShellFolder::createFile(const QString& name, bool overwrite, const char* data, int size)
{
    bool result = false;

    QString path = d->m_path + QLatin1Char('/') + name;

    DWORD creation = overwrite ? CREATE_ALWAYS : OPEN_ALWAYS;
    HANDLE file = CreateFile((LPCWSTR)path.utf16(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, creation, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file != INVALID_HANDLE_VALUE)
    {
        if (data && size > 0 && (overwrite || GetLastError() == 0))
        {
            DWORD written;
            result = WriteFile(file, data, size, &written, nullptr);
        }
        else
        {
            result = true;
        }

        CloseHandle(file);
    }

    return result;
}

ShellItem ShellFolder::childItem(const QString& name)
{
    ShellItem result;

    HRESULT hr = d->m_folder->ParseDisplayName((HWND)parent()->effectiveWinId(), nullptr, (LPWSTR)name.utf16(), nullptr, &result.d->m_pidl, nullptr);

    if (SUCCEEDED(hr)) d->readItemProperties(result);

    return result;
}

// Formats the error code reported by ShellExecuteEx for display to the user. The shell
// returns SE_ERR_ACCESSDENIED (ERROR_ACCESS_DENIED) for many unrelated failures, so the
// numeric code is always included.
static QString shellExecuteErrorText(DWORD error)
{
    if (error == 0) return QCoreApplication::translate("ShellFolder", "The system did not report a reason.");

    const QString code = QStringLiteral("0x%1").arg(error, 8, 16, QLatin1Char('0'));

    wchar_t* message = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                       error,
                       0,
                       (LPWSTR)&message,
                       0,
                       nullptr)
            == 0
        || message == nullptr)
    {
        return code;
    }

    const QString text = QString::fromWCharArray(message).trimmed();
    LocalFree(message);

    return text.isEmpty() ? code : QStringLiteral("%1 (%2)").arg(text, code);
}

// Returns the working directory to pass to ShellExecuteEx. lpDirectory is used verbatim and
// must be a real directory: virtual folders (This PC for example) have paths in the "::{GUID}"
// form, and a NULL value makes the shell fall back to the current working directory of the
// process, which makes the call fail when that directory is no longer available.
// The returned pointer refers to 'directory', which must outlive the call.
static LPCWSTR shellExecuteDirectory(QString& directory, const QString& path)
{
    directory = QDir::toNativeSeparators(path);

    return QFileInfo(directory).isDir() ? (LPCWSTR)directory.utf16() : nullptr;
}

bool ShellFolder::executeItem(const ShellItem& item)
{
    LPITEMIDLIST absolutePidl = ILCombine(d->m_pidl, item.d->m_pidl);

    QString directory;

    SHELLEXECUTEINFO info = {0};
    info.cbSize = sizeof(info);
    // SEE_MASK_FLAG_NO_UI keeps the shell from showing its own, misleading error dialog;
    // the failure is reported below instead.
    info.fMask = SEE_MASK_IDLIST | SEE_MASK_FLAG_NO_UI;
    info.hwnd = (HWND)parent()->effectiveWinId();
    info.lpVerb = nullptr;
    info.lpParameters = nullptr;
    info.lpDirectory = shellExecuteDirectory(directory, d->m_path);
    info.nShow = SW_SHOWNORMAL;
    info.lpIDList = absolutePidl;

    bool result = ShellExecuteEx(&info);
    DWORD error = result ? ERROR_SUCCESS : GetLastError();

    CoTaskMemFree(absolutePidl);

    if (!result)
    {
        qWarning("ShellFolder::executeItem: ShellExecuteEx failed for '%s' (error %lu)", qPrintable(item.name()), (unsigned long)error);

        QMessageBox::warning(parent(), tr("Cannot open item"), tr("Unable to open '%1'.\n\n%2").arg(item.name(), shellExecuteErrorText(error)));
    }

    return result;
}

ShellSelection::MenuCommand ShellFolder::showContextMenu(const QPoint& pos, ShellSelection::Flags flags)
{
    ShellSelection::MenuCommand result = ShellSelection::NoCommand;

    ShellItem item;
    ShellFolder* parentFolder = d->createParentFolder(item);

    if (parentFolder)
    {
        QList<ShellItem> items;
        items.append(item);

        ShellSelection selection(parentFolder, items, parent());
        result = selection.showContextMenu(pos, flags);

        delete parentFolder;
    }

    return result;
}

bool ShellFolder::invokeCommand(const char* verb)
{
    bool result = false;

    ShellItem item;
    ShellFolder* parentFolder = d->createParentFolder(item);

    if (parentFolder)
    {
        QList<ShellItem> items;
        items.append(item);

        ShellSelection selection(parentFolder, items, parent());
        result = selection.invokeCommand(verb);

        delete parentFolder;
    }

    return result;
}

bool ShellFolder::search()
{
    return SHFindFiles(d->m_pidl, nullptr);
}

bool ShellFolder::explore()
{
    QString directory;

    SHELLEXECUTEINFO info = {0};
    info.cbSize = sizeof(info);
    // SEE_MASK_FLAG_NO_UI keeps the shell from showing its own, misleading error dialog;
    // the failure is reported below instead.
    info.fMask = SEE_MASK_IDLIST | SEE_MASK_FLAG_NO_UI;
    info.hwnd = (HWND)parent()->effectiveWinId();
    info.lpVerb = nullptr;
    info.lpParameters = nullptr;
    info.lpDirectory = shellExecuteDirectory(directory, d->m_path);
    info.nShow = SW_SHOWNORMAL;
    info.lpIDList = d->m_pidl;

    bool result = ShellExecuteEx(&info);
    DWORD error = result ? ERROR_SUCCESS : GetLastError();

    if (!result)
    {
        // Fall back to the context menu handler of the shell, which is already used for the
        // other verbs and also works for folders without a filesystem path.
        if (invokeCommand("explore")) return true;

        qWarning("ShellFolder::explore: ShellExecuteEx failed for '%s' (error %lu)", qPrintable(d->m_path), (unsigned long)error);

        QMessageBox::warning(parent(),
                             tr("Cannot explore folder"),
                             tr("Unable to open the folder in Windows Explorer.\n\n%1").arg(shellExecuteErrorText(error)));
    }

    return result;
}

QString ShellFolder::toolTip(const ShellItem& item)
{
    QString result;

    IQueryInfo* queryInfo;
    HRESULT hr = d->m_folder->GetUIObjectOf((HWND)parent()->effectiveWinId(), 1, (LPCITEMIDLIST*)&item.d->m_pidl, IID_IQueryInfo, nullptr, (void**)&queryInfo);

    if (SUCCEEDED(hr))
    {
        wchar_t* tip;
        hr = queryInfo->GetInfoTip(QITIPF_DEFAULT, &tip);

        if (SUCCEEDED(hr))
        {
            result = QString::fromWCharArray(tip);

            CoTaskMemFree(tip);
        }
        queryInfo->Release();
    }

    return result;
}

bool ShellFolder::startWatching()
{
    auto watcher = new ChangeNotifyWatcher(d->m_pidl, SHCNE_DISKEVENTS, this);

    if (watcher->isValid())
    {
        connect(watcher, SIGNAL(changeNotify(int, void*, void*)), this, SLOT(changeNotify(int, void*, void*)));
        return true;
    }

    return false;
}

void ShellFolder::changeNotify(int eventType, void* arg1, void* arg2)
{
    if (eventType == SHCNE_UPDATEDIR && ILIsEqual(d->m_pidl, (LPITEMIDLIST)arg1))
    {
        emit folderUpdated();
        return;
    }

    if (eventType == SHCNE_CREATE || eventType == SHCNE_MKDIR)
    {
        LPITEMIDLIST pidl = ILFindChild(d->m_pidl, (LPITEMIDLIST)arg1);
        if (pidl && !ILIsEmpty(pidl))
        {
            ShellItem item = d->makeRealNotifyItem(pidl);
            if (item.isValid()) emit itemChanged(ItemChange(ItemChange::ItemAdded, item));
            return;
        }
    }

    if (eventType == SHCNE_DELETE || eventType == SHCNE_RMDIR)
    {
        LPITEMIDLIST pidl = ILFindChild(d->m_pidl, (LPITEMIDLIST)arg1);
        if (pidl && !ILIsEmpty(pidl))
        {
            ShellItem item = d->makeNotifyItem(pidl);
            emit itemChanged(ItemChange(ItemChange::ItemRemoved, item));
            return;
        }
    }

    if (eventType == SHCNE_UPDATEITEM || eventType == SHCNE_UPDATEDIR)
    {
        LPITEMIDLIST pidl = ILFindChild(d->m_pidl, (LPITEMIDLIST)arg1);
        if (pidl && !ILIsEmpty(pidl))
        {
            ShellItem item = d->makeRealNotifyItem(pidl);
            if (item.isValid()) emit itemChanged(ItemChange(ItemChange::ItemUpdated, item));
            return;
        }
    }

    if (eventType == SHCNE_RENAMEITEM || eventType == SHCNE_RENAMEFOLDER)
    {
        LPITEMIDLIST fromPidl = ILFindChild(d->m_pidl, (LPITEMIDLIST)arg1);
        LPITEMIDLIST toPidl = ILFindChild(d->m_pidl, (LPITEMIDLIST)arg2);
        if (fromPidl && !ILIsEmpty(fromPidl) && toPidl && !ILIsEmpty(toPidl))
        {
            ShellItem fromItem = d->makeNotifyItem(fromPidl);
            ShellItem toItem = d->makeRealNotifyItem(toPidl);
            if (toItem.isValid()) emit itemChanged(ItemChange(ItemChange::ItemRenamed, fromItem, toItem));
            return;
        }
    }
}

ItemChange::ItemChange(ItemChange::Type type, const ShellItem& item1, const ShellItem& item2 /*= ShellItem()*/)
    : m_type(type)
    , m_item1(item1)
    , m_item2(item2)
{
}

ItemChange::~ItemChange()
{
}

ItemChange::ItemChange(const ItemChange& other)
    : m_type(other.m_type)
    , m_item1(other.m_item1)
    , m_item2(other.m_item2)
{
}

ItemChange& ItemChange::operator=(const ItemChange& other)
{
    m_type = other.m_type;
    m_item1 = other.m_item1;
    m_item2 = other.m_item2;
    return *this;
}
