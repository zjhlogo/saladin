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

#include "shellbase_p.h"

#include <shlwapi.h>

ShellBasePrivate::ShellBasePrivate()
    : m_pidl(nullptr)
    , m_folder(nullptr)
{
}

ShellBasePrivate::~ShellBasePrivate()
{
    if (m_pidl)
    {
        CoTaskMemFree(m_pidl);
        m_pidl = nullptr;
    }

    if (m_folder)
    {
        m_folder->Release();
        m_folder = nullptr;
    }
}

QString ShellBasePrivate::displayName(LPITEMIDLIST pidl, int flags)
{
    QString result;

    STRRET strret;
    HRESULT hr = m_folder->GetDisplayNameOf(pidl, flags, &strret);

    if (SUCCEEDED(hr))
    {
        wchar_t buffer[MAX_PATH];
        hr = StrRetToBuf(&strret, pidl, buffer, MAX_PATH);

        if SUCCEEDED (hr) result = QString::fromUtf16((const ushort*)buffer);
    }

    return result;
}

QPixmap ShellBasePrivate::extractIcon(LPITEMIDLIST pidl)
{
    QPixmap result;

    LPITEMIDLIST absolutePidl = ILCombine(m_pidl, pidl);

    // Filesystem items are resolved by path, which skips the slower PIDL binding
    // done for SHGFI_PIDL. SHGFI_SYSICONINDEX makes SHGetFileInfo resolve the icon
    // through the system image list (the shell's icon cache shared with Explorer),
    // so re-requesting the icon of the same file is served from the cache instead
    // of re-extracting it from the file.
    wchar_t path[MAX_PATH];
    bool hasPath = SHGetPathFromIDListW(absolutePidl, path);

    DWORD flags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS;
    if (!hasPath) flags |= SHGFI_PIDL;

    SHFILEINFO info;
    DWORD_PTR status = SHGetFileInfo(hasPath ? reinterpret_cast<LPCWSTR>(path) : reinterpret_cast<LPCWSTR>(absolutePidl), 0, &info, sizeof(info), flags);

    CoTaskMemFree(absolutePidl);

    if (status)
    {
        result = QPixmap::fromImage(QImage::fromHICON(info.hIcon));
        DestroyIcon(info.hIcon);
    }

    return result;
}
