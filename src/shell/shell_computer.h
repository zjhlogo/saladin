// \file shell_computer.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "shell_drive.h"
#include "shell_selection.h"

#include <QObject>

class ShellFolder;
class ShellComputerPrivate;
class ChangeNotifyWatcher;

class ShellComputer : public QObject
{
    Q_OBJECT
public:
    explicit ShellComputer(QWidget* parent);
    ~ShellComputer() override;

public:
    QList<ShellDrive> listDrives();

    ShellFolder* openRootFolder(const ShellDrive& drive);

    ShellSelection::MenuCommand showContextMenu(const ShellDrive& drive, const QPoint& pos, ShellSelection::Flags flags);

    bool startWatching();

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>(QObject::parent()); }

signals:
    void driveChanged(const ShellDrive& drive);
    void computerUpdated();

private slots:
    void changeNotify(int eventType, void* arg1, void* arg2);

private:
    ShellComputerPrivate* d;

    ChangeNotifyWatcher* m_watcher{};

    friend class ShellDropData;
};
