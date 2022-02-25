/**************************************************************************
 * This file is part of the Saladin program
 * Copyright (C) 2011-2017 Michał Męciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "shellcomputer.h"
#include "changenotifywatcher_p.h"
#include "shellcomputer_p.h"
#include "shelldrive_p.h"
#include "shellfolder.h"
#include "shellfolder_p.h"
#include "shellitem_p.h"

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
    ShellFolder* folder = new ShellFolder(d->m_folder, parent());

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
    ChangeNotifyWatcher* watcher = new ChangeNotifyWatcher(d->m_pidl, SHCNE_GLOBALEVENTS, this);

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
