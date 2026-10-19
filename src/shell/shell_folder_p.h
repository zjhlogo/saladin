// \file shell_folder_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_base_p.h"

#include <shlobj.h>

class ShellFolder;

class ShellFolderPrivate : public ShellBasePrivate
{
public:
    ShellFolderPrivate();
    ~ShellFolderPrivate();

public:
    void readProperties();

    ShellItem makeItem(LPITEMIDLIST pidl);
    ShellItem makeNotifyItem(LPITEMIDLIST pidl);
    ShellItem makeRealNotifyItem(LPITEMIDLIST pidl);

    void readItemProperties(ShellItem& item);

    ShellFolder* createParentFolder(ShellItem& item);

    qint64 calculateSize(IShellFolder* parentFolder, LPITEMIDLIST pidl);

public:
    ShellFolder* q;

    QString m_path;

    QString m_user;
    QString m_password;

    bool m_hasParent;
};
