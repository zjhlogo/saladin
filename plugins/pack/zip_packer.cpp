// \file zip_packer.cpp
// \author zhangjianhua
// \date 2026/10/23

#include "zip_packer.h"

#include <shlobj.h>
#include <shobjidl.h>
#include <windows.h>
#include <wrl/client.h>

#include <QFile>
#include <QVector>
#include <QWidget>

using Microsoft::WRL::ComPtr;

namespace
{
    QString toWindowsPath(const QString& path)
    {
        QString result = path;
        result.replace(QLatin1Char('/'), QLatin1Char('\\'));
        return result;
    }

    PIDLIST_ABSOLUTE parseDisplayName(const QString& path)
    {
        const QString windowsPath = toWindowsPath(path);

        PIDLIST_ABSOLUTE result = nullptr;
        if (FAILED(SHParseDisplayName(reinterpret_cast<PCWSTR>(windowsPath.utf16()), nullptr, &result, 0, nullptr))) return nullptr;

        return result;
    }
} // namespace

bool Pack::writeEmptyZip(const QString& filePath)
{
    // ZIP 的“中央目录结束记录”：不带任何条目的合法空压缩包。
    static constexpr char ZIP_END_OF_CENTRAL_DIRECTORY[22] = {'P', 'K', 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    return file.write(ZIP_END_OF_CENTRAL_DIRECTORY, sizeof(ZIP_END_OF_CENTRAL_DIRECTORY)) == sizeof(ZIP_END_OF_CENTRAL_DIRECTORY);
}

bool Pack::addToZip(QWidget* owner, const QString& sourceFolderPath, const QStringList& itemNames, const QString& zipPath)
{
    if (itemNames.isEmpty()) return false;

    HWND hwnd = owner ? reinterpret_cast<HWND>(owner->winId()) : nullptr;

    bool result = false;

    PIDLIST_ABSOLUTE zipPidl = parseDisplayName(zipPath);
    if (!zipPidl) return false;

    ComPtr<IShellFolder> zipFolder;
    HRESULT hr = SHBindToObject(nullptr, zipPidl, nullptr, IID_PPV_ARGS(&zipFolder));

    if (SUCCEEDED(hr))
    {
        ComPtr<IDropTarget> dropTarget;
        hr = zipFolder->CreateViewObject(hwnd, IID_PPV_ARGS(&dropTarget));

        if (SUCCEEDED(hr))
        {
            ComPtr<IDataObject> dataObject;

            PIDLIST_ABSOLUTE folderPidl = parseDisplayName(sourceFolderPath);

            if (folderPidl)
            {
                ComPtr<IShellFolder> sourceFolder;
                hr = SHBindToObject(nullptr, folderPidl, nullptr, IID_PPV_ARGS(&sourceFolder));

                if (SUCCEEDED(hr))
                {
                    QVector<PIDLIST_RELATIVE> childPidls;
                    childPidls.reserve(itemNames.count());

                    foreach (QString name, itemNames)
                    {
                        PIDLIST_RELATIVE childPidl = nullptr;
                        if (SUCCEEDED(sourceFolder->ParseDisplayName(hwnd, nullptr, (LPWSTR)name.utf16(), nullptr, &childPidl, nullptr)))
                            childPidls.append(childPidl);
                    }

                    if (childPidls.count() == itemNames.count())
                    {
                        hr = sourceFolder->GetUIObjectOf(hwnd,
                                                         (UINT)childPidls.count(),
                                                         (PCUITEMID_CHILD_ARRAY)childPidls.data(),
                                                         IID_IDataObject,
                                                         nullptr,
                                                         reinterpret_cast<void**>(dataObject.GetAddressOf()));
                    }
                    else
                    {
                        hr = E_FAIL;
                    }

                    foreach (PIDLIST_RELATIVE childPidl, childPidls)
                        CoTaskMemFree(childPidl);
                }

                CoTaskMemFree(folderPidl);
            }
            else
            {
                hr = E_FAIL;
            }

            if (SUCCEEDED(hr))
            {
                POINTL point = {0, 0};
                DWORD possibleEffect = DROPEFFECT_COPY;

                DWORD dragEffect = possibleEffect;
                hr = dropTarget->DragEnter(dataObject.Get(), MK_LBUTTON, point, &dragEffect);

                if (SUCCEEDED(hr) && (dragEffect & possibleEffect) != 0)
                {
                    DWORD dropEffect = possibleEffect;
                    hr = dropTarget->Drop(dataObject.Get(), MK_LBUTTON, point, &dropEffect);

                    if (SUCCEEDED(hr))
                    {
                        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, zipPidl, nullptr);
                        result = true;
                    }
                }
                else
                {
                    dropTarget->DragLeave();
                }
            }
        }
    }

    CoTaskMemFree(zipPidl);

    return result;
}
