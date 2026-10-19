// \file shell_drive.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QSharedData>

class ShellDrivePrivate;

class ShellDrive
{
public:
    ShellDrive();
    ~ShellDrive();

    ShellDrive(const ShellDrive& other);
    ShellDrive& operator=(const ShellDrive& other);

public:
    bool isValid() const;

    char letter() const;
    QString name() const;
    QPixmap icon() const;

    bool getFreeSpace(qint64* free, qint64* total) const;

public:
    friend bool operator==(const ShellDrive& lhs, const ShellDrive& rhs);
    friend bool operator!=(const ShellDrive& lhs, const ShellDrive& rhs);

private:
    QSharedDataPointer<ShellDrivePrivate> d;

    friend class ShellComputer;
    friend class ShellComputerPrivate;
    friend class ShellDropData;
};

Q_DECLARE_TYPEINFO(ShellDrive, Q_MOVABLE_TYPE);
