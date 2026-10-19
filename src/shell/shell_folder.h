// \file shell_folder.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "shell_item.h"
#include "shell_selection.h"

#include <QObject>

class ShellFolderPrivate;
class ShellPidl;
class ItemChange;

class ShellFolder : public QObject
{
    Q_OBJECT
public:
    enum Flag
    {
        IncludeHidden = 1
    };
    Q_DECLARE_FLAGS(Flags, Flag);

public:
    ShellFolder(const QString& path, QWidget* parent);
    ShellFolder(const ShellPidl& pidl, QWidget* parent);
    ~ShellFolder() override;

public:
    bool isValid() const;
    QString path() const;
    bool hasParent() const;

    ShellItem::Attributes attributes();
    QString name();

    ShellPidl pidl() const;

    QString user() const;
    QString password() const;

    QList<ShellItem> listItems(Flags flags);

    bool extractIcon(ShellItem& item);

    bool calculateSize(ShellItem& item);

    bool setItemName(ShellItem& item, const QString& name);

    QString itemPath(const ShellItem& item) const;
    ShellPidl itemPidl(const ShellItem& item) const;

    bool isEqual(const ShellFolder* other) const;

    ShellFolder* clone();

    ShellFolder* openFolder(const ShellItem& item);
    ShellFolder* parentFolder(ShellItem& item);
    ShellFolder* rootFolder();

    ShellFolder* browseFolder();

    bool canCreateFolder();
    bool createFolder(const QString& name);

    bool createFile(const QString& name, bool overwrite, const char* data = nullptr, int size = 0);

    ShellItem childItem(const QString& name);

    bool executeItem(const ShellItem& item);

    ShellSelection::MenuCommand showContextMenu(const QPoint& pos, ShellSelection::Flags flags);

    bool invokeCommand(const char* verb);

    bool search();
    bool explore();

    QString toolTip(const ShellItem& item);

    bool startWatching();

public:
    static ShellPidl defaultFolder();

    static ShellPidl browseFolder(QWidget* parent, const QString& title, const ShellPidl& startPidl);

public: // overrides
    QWidget* parent() const { return qobject_cast<QWidget*>(QObject::parent()); }

signals:
    void itemChanged(const ItemChange& change);
    void folderUpdated();

private slots:
    void changeNotify(int eventType, void* arg1, void* arg2);

private:
    ShellFolder(IShellFolder* folder, QWidget* parent);

private:
    ShellFolderPrivate* d;

    friend class ShellFolderPrivate;
    friend class ShellComputer;
    friend class ShellSelection;
    friend class ShellSelectionPrivate;
    friend class ShellDropData;
};

class ItemChange
{
public:
    enum Type
    {
        ItemAdded,
        ItemRemoved,
        ItemUpdated,
        ItemRenamed
    };

public:
    ItemChange(Type type, const ShellItem& item1, const ShellItem& item2 = ShellItem());
    ~ItemChange();

public:
    ItemChange(const ItemChange& other);
    ItemChange& operator=(const ItemChange& other);

public:
    Type type() const { return m_type; }

    const ShellItem& item1() const { return m_item1; }
    ShellItem& item1() { return m_item1; }

    const ShellItem& item2() const { return m_item2; }
    ShellItem& item2() { return m_item2; }

private:
    Type m_type;
    ShellItem m_item1;
    ShellItem m_item2;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShellFolder::Flags);
