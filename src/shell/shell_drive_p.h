// \file shell_drive_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_drive.h"

#include <shlobj.h>

class ShellDrivePrivate : public QSharedData
{
public:
    ShellDrivePrivate();
    ShellDrivePrivate(const ShellDrivePrivate& other);
    ~ShellDrivePrivate();

public:
    LPITEMIDLIST m_pidl;

    char m_letter;
    QString m_name;
    QPixmap m_icon;
};
