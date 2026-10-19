// \file shell_item.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QSharedData>

class ShellItemPrivate;

class ShellItem
{
public:
    enum Attribute
    {
        // shell attributes
        Folder = 1,
        Stream = 2,
        FileSystem = 4,
        CanRename = 8,
        // file system attributes
        ReadOnly = 0x100,
        Hidden = 0x200,
        System = 0x400,
        Directory = 0x800,
        Archive = 0x1000,
        Compressed = 0x2000,
        Encrypted = 0x4000,
        ReparsePoint = 0x8000,
    };
    Q_DECLARE_FLAGS(Attributes, Attribute);

    enum StateFlag
    {
        HasProperties = 1,
        HasExtractedIcon = 2,
        HasCalculatedSize = 4,
        IsSelected = 8
    };
    Q_DECLARE_FLAGS(State, StateFlag);

public:
    ShellItem();
    ~ShellItem();

    ShellItem(const ShellItem& other);
    ShellItem& operator=(const ShellItem& other);

public:
    bool isValid() const;

    QString name() const;
    QString ext() const;
    quint64 size() const;
    QString humanReadableSize() const;
    QDateTime lastModified() const;
    QPixmap icon() const;

    Attributes attributes() const;
    State state() const;

    void setSelected(bool selected);
    bool isSelected() const;

public:
    friend bool operator==(const ShellItem& lhs, const ShellItem& rhs);
    friend bool operator!=(const ShellItem& lhs, const ShellItem& rhs);

private:
    QSharedDataPointer<ShellItemPrivate> d;

    friend class ShellFolder;
    friend class ShellFolderPrivate;
    friend class ShellSelection;
    friend class ShellSelectionPrivate;
    friend class ShellComputer;
    friend class ShellDropData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShellItem::Attributes)
Q_DECLARE_OPERATORS_FOR_FLAGS(ShellItem::State)

Q_DECLARE_TYPEINFO(ShellItem, Q_MOVABLE_TYPE);
