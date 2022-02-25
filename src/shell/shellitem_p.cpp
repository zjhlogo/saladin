/**************************************************************************
 * This file is part of the Saladin program
 * Copyright (C) 2011-2017 Michał Męciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "shellitem_p.h"

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
