/**************************************************************************
* This file is part of the Saladin program
* Copyright (C) 2011 Michał Męciński
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

#ifndef SHELLCOMPUTER_P_H
#define SHELLCOMPUTER_P_H

#include "shellbase_p.h"

#include <shlobj.h>

class ShellComputer;
class ShellDrive;

class ShellComputerPrivate : public ShellBasePrivate
{
public:
    ShellComputerPrivate();
    ~ShellComputerPrivate();

public:
    ShellDrive makeDrive( LPITEMIDLIST pidl );
    ShellDrive makeRealNotifyDrive( LPITEMIDLIST pidl );

    void readDriveProperties( ShellDrive& drive );

public:
    ShellComputer* q;
};

#endif
