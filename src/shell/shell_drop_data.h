// \file shell_drop_data.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QObject>

class ShellDropDataPrivate;
class ShellFolder;
class ShellComputer;
class ShellItem;
class ShellDrive;

class ShellDropData : public QObject
{
    Q_OBJECT
public:
    ShellDropData(QDropEvent* e, ShellFolder* folder, QWidget* parent);
    ShellDropData(QDropEvent* e, ShellComputer* computer, QWidget* parent);
    ~ShellDropData() override;

public:
    bool isValid() const;

    Qt::DropAction dropAction() const;

    bool dragToFolder(QDropEvent* e);
    bool dragToParent(QDropEvent* e);
    bool dragToItem(QDropEvent* e, const ShellItem& item);
    bool dragToDrive(QDropEvent* e, const ShellDrive& drive);

    bool drop();

public:
    static void registerDropTarget(QWindow* window);
    static void unregisterDropTarget(QWindow* window);

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>(QObject::parent()); }

private:
    ShellDropDataPrivate* d;

    friend class ShellSelection;
};
