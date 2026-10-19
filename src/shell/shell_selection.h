// \file shell_selection.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QObject>

class ShellFolder;
class ShellItem;
class ShellDropData;
class ShellSelectionPrivate;

class ShellSelection : public QObject
{
    Q_OBJECT
public:
    enum TransferType
    {
        Copy,
        Move
    };

    enum Flag
    {
        ForceOverwrite = 1,
        ForceDelete = 2,
        DeletePermanently = 4,
        CanOpen = 8,
        CanRename = 16,
    };
    Q_DECLARE_FLAGS(Flags, Flag);

    enum MenuCommand
    {
        NoCommand,
        InternalCommand,
        Open,
        Rename
    };

public:
    ShellSelection(ShellFolder* folder, const QList<ShellItem>& items, QWidget* parent);
    ~ShellSelection() override;

public:
    ShellFolder* folder() const;
    QList<ShellItem> items() const;

    bool canTransferTo(ShellFolder* targetFolder, TransferType type);
    bool transferTo(ShellFolder* targetFolder, TransferType type, Flags flags, QStringList newNames);

    bool canDragDropTo(ShellFolder* targetFolder, TransferType type);
    bool dragDropTo(ShellFolder* targetFolder, TransferType type);

    bool canDelete();
    bool deleteSelection(Flags flags);

    MenuCommand showContextMenu(const QPoint& pos, Flags flags);

    bool invokeCommand(const char* verb);

    bool doDragDrop();

    static ShellSelection* draggedSelection(ShellDropData* data);

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>(QObject::parent()); }

private:
    ShellSelectionPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShellSelection::Flags)
