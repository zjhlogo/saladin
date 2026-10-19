// \file shell_selection_p.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <shlobj.h>

class ShellSelectionPrivate
{
public:
    ShellSelectionPrivate();
    ~ShellSelectionPrivate();

public:
    bool dragDropHelper(ShellFolder* targetFolder, ShellSelection::TransferType type, bool doDrop);

public:
    ShellSelection* q;

    ShellFolder* m_sourceFolder;
    QList<ShellItem> m_sourceItems;

    static ShellSelection* m_dragSelection;
    static IDataObject* m_dragObject;
};
