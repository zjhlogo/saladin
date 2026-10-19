// \file shell_pidl.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "shell_pidl.h"
#include "shell_pidl_p.h"

ShellPidlPrivate::ShellPidlPrivate()
    : m_attributes(0)
{
}

ShellPidlPrivate::ShellPidlPrivate(const ShellPidlPrivate& other)
    : QSharedData(other)
    , m_data(other.m_data)
    , m_path(other.m_path)
    , m_attributes(other.m_attributes)
{
}

ShellPidlPrivate::~ShellPidlPrivate()
{
}

ShellPidl::ShellPidl()
    : d(new ShellPidlPrivate())
{
}

ShellPidl::~ShellPidl()
{
}

ShellPidl::ShellPidl(const ShellPidl& other)
    : d(other.d)
{
}

ShellPidl& ShellPidl::operator=(const ShellPidl& other)
{
    d = other.d;
    return *this;
}

bool ShellPidl::isValid() const
{
    return !d->m_data.isEmpty();
}

QString ShellPidl::path() const
{
    return d->m_path;
}

ShellItem::Attributes ShellPidl::attributes() const
{
    return d->m_attributes;
}

bool operator==(const ShellPidl& lhs, const ShellPidl& rhs)
{
    return ILIsEqual(lhs.d->pidl(), rhs.d->pidl());
}

bool operator!=(const ShellPidl& lhs, const ShellPidl& rhs)
{
    return !(lhs == rhs);
}

QDataStream& operator<<(QDataStream& stream, const ShellPidl& pidl)
{
    return stream << pidl.d->m_data << pidl.d->m_path;
}

QDataStream& operator>>(QDataStream& stream, ShellPidl& pidl)
{
    return stream >> pidl.d->m_data >> pidl.d->m_path;
}

void ShellPidl::registerMetaType()
{
    qRegisterMetaType<ShellPidl>("ShellPidl");
}
