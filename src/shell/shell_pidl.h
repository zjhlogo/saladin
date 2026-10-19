// \file shell_pidl.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_item.h"

#include <QSharedData>

class ShellPidlPrivate;

class ShellPidl
{
public:
    ShellPidl();
    ~ShellPidl();

    ShellPidl(const ShellPidl& other);
    ShellPidl& operator=(const ShellPidl& other);

public:
    bool isValid() const;

    QString path() const;

    ShellItem::Attributes attributes() const;

public:
    static void registerMetaType();

public:
    friend bool operator==(const ShellPidl& lhs, const ShellPidl& rhs);
    friend bool operator!=(const ShellPidl& lhs, const ShellPidl& rhs);

    friend QDataStream& operator<<(QDataStream& stream, const ShellPidl& pidl);
    friend QDataStream& operator>>(QDataStream& stream, ShellPidl& pidl);

private:
    QSharedDataPointer<ShellPidlPrivate> d;

    friend class ShellFolder;
    friend class StreamDevice;
};

Q_DECLARE_METATYPE(ShellPidl)

Q_DECLARE_TYPEINFO(ShellPidl, Q_MOVABLE_TYPE);
