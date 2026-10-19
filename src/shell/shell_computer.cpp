// \file shell_computer.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "shell_computer.h"
#include "change_notify_watcher_p.h"
#include "shell_computer_p.h"
#include "shell_drive_p.h"
#include "shell_folder.h"
#include "shell_folder_p.h"
#include "shell_item_p.h"

#include <shlwapi.h>

ShellComputer::ShellComputer(QWidget* parent)
    : QObject(parent)
    , d(new ShellComputerPrivate())
{
    d->q = this;

    IShellFolder* desktopFolder;
    HRESULT hr = SHGetDesktopFolder(&desktopFolder);

    if (SUCCEEDED(hr))
    {
        hr = SHGetKnownFolderIDList(FOLDERID_ComputerFolder, 0, nullptr, &d->m_pidl);

        if (SUCCEEDED(hr)) desktopFolder->BindToObject(d->m_pidl, nullptr, IID_PPV_ARGS(&d->m_folder));

        desktopFolder->Release();
    }
}

ShellComputer::~ShellComputer()
{
    delete d;
}

ShellComputerPrivate::ShellComputerPrivate()
    : q(nullptr)
{
}

ShellComputerPrivate::~ShellComputerPrivate()
{
}

QList<ShellDrive> ShellComputer::listDrives()
{
    QList<ShellDrive> result;

    IEnumIDList* enumerator;
    HRESULT hr = d->m_folder->EnumObjects((HWND)parent()->effectiveWinId(), SHCONTF_STORAGE, &enumerator);

    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        while (enumerator->Next(1, &pidl, nullptr) == S_OK)
        {
            ShellDrive drive = d->makeDrive(pidl);
            if (drive.isValid()) result.append(drive);
        }

        enumerator->Release();
    }

    return result;
}

ShellDrive ShellComputerPrivate::makeDrive(LPITEMIDLIST pidl)
{
    ShellDrive drive;
    drive.d->m_pidl = pidl;

    if (!readDriveProperties(drive)) return ShellDrive();

    return drive;
}

ShellDrive ShellComputerPrivate::makeRealNotifyDrive(LPITEMIDLIST pidl)
{
    ShellDrive drive;

    HRESULT hr = SHGetRealIDL(m_folder, pidl, &drive.d->m_pidl);

    if (SUCCEEDED(hr)) readDriveProperties(drive);

    return drive;
}

bool ShellComputerPrivate::readDriveProperties(ShellDrive& drive)
{
    QString path = displayName(drive.d->m_pidl, SHGDN_FORPARSING);

    if (path.length() != 3 || path.at(1) != ':' || path.at(2) != '\\') return false;

    drive.d->m_name = displayName(drive.d->m_pidl, SHGDN_INFOLDER | SHGDN_FOREDITING);

    drive.d->m_letter = path.at(0).toUpper().toLatin1();

    drive.d->m_icon = extractIcon(drive.d->m_pidl);

    return true;
}

ShellFolder* ShellComputer::openRootFolder(const ShellDrive& drive)
{
    ShellFolder* result = nullptr;

    IShellFolder* folder;
    HRESULT hr = SHBindToObject(d->m_folder, drive.d->m_pidl, nullptr, IID_PPV_ARGS(&folder));

    if (SUCCEEDED(hr)) result = new ShellFolder(folder, parent());

    return result;
}

ShellSelection::MenuCommand ShellComputer::showContextMenu(const ShellDrive& drive, const QPoint& pos, ShellSelection::Flags flags)
{
    ShellSelection::MenuCommand result;

    d->m_folder->AddRef();
    auto folder = new ShellFolder(d->m_folder, parent());

    ShellItem item;
    item.d->m_pidl = ILClone(drive.d->m_pidl);

    QList<ShellItem> items;
    items.append(item);

    ShellSelection selection(folder, items, parent());
    result = selection.showContextMenu(pos, flags);

    delete folder;

    return result;
}

bool ShellComputer::startWatching()
{
    auto watcher = new ChangeNotifyWatcher(d->m_pidl, SHCNE_GLOBALEVENTS, this);

    if (watcher->isValid())
    {
        connect(watcher, SIGNAL(changeNotify(int, void*, void*)), this, SLOT(changeNotify(int, void*, void*)));
        return true;
    }

    return false;
}

void ShellComputer::changeNotify(int eventType, void* arg1, void* /*arg2*/)
{
    if (eventType == SHCNE_DRIVEADD || eventType == SHCNE_DRIVEREMOVED)
    {
        emit computerUpdated();
        return;
    }

    if (eventType == SHCNE_MEDIAINSERTED || eventType == SHCNE_MEDIAREMOVED || eventType == SHCNE_UPDATEIMAGE)
    {
        LPITEMIDLIST pidl = ILFindChild(d->m_pidl, (LPITEMIDLIST)arg1);
        if (pidl)
        {
            ShellDrive drive = d->makeRealNotifyDrive(pidl);
            emit driveChanged(drive);
            return;
        }
    }
}
