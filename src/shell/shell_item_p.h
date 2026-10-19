// \file shell_item_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_item.h"

#include <shlobj.h>

class ShellItemPrivate : public QSharedData
{
public:
    ShellItemPrivate();
    ShellItemPrivate(const ShellItemPrivate& other);
    ~ShellItemPrivate();

public:
    LPITEMIDLIST m_pidl;

    QString m_name;
    quint64 m_size;
    QDateTime m_modified;
    QPixmap m_icon;

    ShellItem::Attributes m_attributes;
    ShellItem::State m_state;
};
