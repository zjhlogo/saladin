// \file shell_base_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <shlobj.h>

class ShellBasePrivate
{
public:
    ShellBasePrivate();
    ~ShellBasePrivate();

public:
    QString displayName(LPITEMIDLIST pidl, int flags);
    QPixmap extractIcon(LPITEMIDLIST pidl);

public:
    LPITEMIDLIST m_pidl{};
    IShellFolder* m_folder{};
};
