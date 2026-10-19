// \file shell_drive.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "shell_drive.h"
#include "shell_drive_p.h"

ShellDrivePrivate::ShellDrivePrivate()
    : m_pidl(nullptr)
{
}

ShellDrivePrivate::ShellDrivePrivate(const ShellDrivePrivate& other)
    : QSharedData(other)
    , m_pidl(ILClone(other.m_pidl))
{
}

ShellDrivePrivate::~ShellDrivePrivate()
{
    if (m_pidl)
    {
        CoTaskMemFree(m_pidl);
        m_pidl = nullptr;
    }
}

ShellDrive::ShellDrive()
    : d(new ShellDrivePrivate())
{
}

ShellDrive::~ShellDrive()
{
}

ShellDrive::ShellDrive(const ShellDrive& other)
    : d(other.d)
{
}

ShellDrive& ShellDrive::operator=(const ShellDrive& other)
{
    d = other.d;
    return *this;
}

bool ShellDrive::isValid() const
{
    return d->m_pidl != nullptr;
}

char ShellDrive::letter() const
{
    return d->m_letter;
}

QString ShellDrive::name() const
{
    return d->m_name;
}

QPixmap ShellDrive::icon() const
{
    return d->m_icon;
}

bool ShellDrive::getFreeSpace(qint64* free, qint64* total) const
{
    QString path = QLatin1Char(d->m_letter) + QLatin1String(":\\");

    ULARGE_INTEGER freeBytes;
    ULARGE_INTEGER totalBytes;

    if (GetDiskFreeSpaceEx((LPCWSTR)path.utf16(), &freeBytes, &totalBytes, nullptr))
    {
        *free = freeBytes.QuadPart;
        *total = totalBytes.QuadPart;
        return true;
    }

    return false;
}

bool operator==(const ShellDrive& lhs, const ShellDrive& rhs)
{
    return ILIsEqual(lhs.d->m_pidl, rhs.d->m_pidl);
}

bool operator!=(const ShellDrive& lhs, const ShellDrive& rhs)
{
    return !(lhs == rhs);
}
