// \file shell_computer_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_base_p.h"

#include <shlobj.h>

class ShellComputer;
class ShellDrive;

class ShellComputerPrivate : public ShellBasePrivate
{
public:
    ShellComputerPrivate();
    ~ShellComputerPrivate();

public:
    ShellDrive makeDrive(LPITEMIDLIST pidl);
    ShellDrive makeRealNotifyDrive(LPITEMIDLIST pidl);

    bool readDriveProperties(ShellDrive& drive);

public:
    ShellComputer* q;
};
