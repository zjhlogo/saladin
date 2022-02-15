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

#include "shellitem.h"
#include "shellitem_p.h"

ShellItem::ShellItem()
    : d(new ShellItemPrivate())
{
}

ShellItem::~ShellItem()
{
}

ShellItem::ShellItem(const ShellItem& other)
    : d(other.d)
{
}

ShellItem& ShellItem::operator=(const ShellItem& other)
{
    d = other.d;
    return *this;
}

bool ShellItem::isValid() const
{
    return d->m_pidl != nullptr;
}

QString ShellItem::name() const
{
    return d->m_name;
}

QString ShellItem::ext() const
{
    if (d->m_attributes.testFlag(ShellItem::Directory)) return "";

    QFileInfo fi(d->m_name);
    return fi.suffix();
}

quint64 ShellItem::size() const
{
    return d->m_size;
}

QString ShellItem::humanReadableSize() const
{
    static const quint64 GB = 1024L * 1024L * 1024L;
    static const quint64 MB = 1024L * 1024L;
    static const quint64 KB = 1024L;

    if (d->m_size >= GB)
    {
        return QString("%1 G").arg(QString::number((double)d->m_size / GB, 10, 1));
    }
    else if (d->m_size >= MB)
    {
        return QString("%1 M").arg(QString::number((double)d->m_size / MB, 10, 1));
    }
    else if (d->m_size >= KB)
    {
        return QString("%1 K").arg(QString::number((double)d->m_size / KB, 10, 1));
    }

    return QString("%1 B").arg(d->m_size);
}

QDateTime ShellItem::lastModified() const
{
    return d->m_modified;
}

QPixmap ShellItem::icon() const
{
    return d->m_icon;
}

ShellItem::Attributes ShellItem::attributes() const
{
    return d->m_attributes;
}

ShellItem::State ShellItem::state() const
{
    return d->m_state;
}

void ShellItem::setSelected(bool selected)
{
    if (selected)
        d->m_state |= IsSelected;
    else
        d->m_state &= ~IsSelected;
}

bool ShellItem::isSelected() const
{
    return d->m_state & IsSelected;
}

bool operator==(const ShellItem& lhs, const ShellItem& rhs)
{
    return ILIsEqual(lhs.d->m_pidl, rhs.d->m_pidl);
}

bool operator!=(const ShellItem& lhs, const ShellItem& rhs)
{
    return !(lhs == rhs);
}
