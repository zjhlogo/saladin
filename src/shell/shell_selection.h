// \file shell_selection.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell_item.h"

#include <QList>
#include <QObject>

class QAction;
class ShellFolder;
class ShellDropData;
class ShellSelectionPrivate;

/**
 * 文件右键菜单的扩展点：返回要追加到原生 Shell 菜单末尾的 Qt 动作。
 *
 * 由插件框架（PluginManager）在启动时设置、退出时传 nullptr 恢复默认；
 * shell 层不依赖插件层。folder 是选中项所在的目录，items 是当前选中项
 * （空白处右键时为目录本身对应的选择）。
 */
using ShellContextMenuProvider = QList<QAction*> (*)(ShellFolder* folder, const QList<ShellItem>& items);

void setShellContextMenuProvider(ShellContextMenuProvider provider);

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
