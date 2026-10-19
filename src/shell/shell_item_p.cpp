// \file shell_item_p.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "shell_item_p.h"

ShellItemPrivate::ShellItemPrivate()
    : m_pidl(nullptr)
    , m_size(0)
    , m_attributes(0)
    , m_state(0)
{
}

ShellItemPrivate::ShellItemPrivate(const ShellItemPrivate& other)
    : QSharedData(other)
    , m_pidl(ILClone(other.m_pidl))
    , m_name(other.m_name)
    , m_size(other.m_size)
    , m_modified(other.m_modified)
    , m_icon(other.m_icon)
    , m_attributes(other.m_attributes)
    , m_state(other.m_state)
{
}

ShellItemPrivate::~ShellItemPrivate()
{
    if (m_pidl)
    {
        CoTaskMemFree(m_pidl);
        m_pidl = nullptr;
    }
}
