// \file shell_pidl_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_pidl.h"

#include <shlobj.h>

class ShellPidlPrivate : public QSharedData
{
public:
    ShellPidlPrivate();
    ShellPidlPrivate(const ShellPidlPrivate& other);
    ~ShellPidlPrivate();

public:
    LPCITEMIDLIST pidl() const { return (LPCITEMIDLIST)m_data.data(); }

public:
    QByteArray m_data;
    QString m_path;
    ShellItem::Attributes m_attributes;
};
