// \file shell_drop_data_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_drive.h"
#include "shell_drop_data.h"
#include "shell_item.h"

class WindowDropTarget;

class ShellDropDataPrivate
{
public:
    enum Target
    {
        NoTarget,
        FolderTarget,
        ParentTarget,
        ItemTarget,
        DriveTarget
    };

public:
    ShellDropDataPrivate();
    ~ShellDropDataPrivate();

public:
    bool dragOver(QDropEvent* e);

    void dragLeave();

public:
    ShellDropData* q;

    IDataObject* m_dataObject;

    Target m_target;

    ShellFolder* m_folder;
    ShellComputer* m_computer;

    ShellItem m_item;
    ShellDrive m_drive;

    bool m_ignoreItem;

    IDropTarget* m_dropTarget;
    LPITEMIDLIST m_targetPidl;

    POINTL m_point;
    DWORD m_keyState;
    DWORD m_possibleEffect;

    bool m_dragEntered;
    Qt::DropAction m_dropAction;

    static WindowDropTarget* m_windowDropTarget;

    static IDataObject* m_dropDataObject;
    static QMimeData* m_dropMimeData;
};
