// \file folder_item_model.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_folder.h"
#include "shell/shell_item.h"

#include <QMainWindow>

class FolderItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Columns
    {
        Column_Name = 0,
        Column_Ext,
        Column_Size,
        Column_LastModified,
        Column_Attributes,
        Num_Column,
    };

public:
    explicit FolderItemModel(QObject* parent);
    ~FolderItemModel();

public:
    void setIncludeHidden(bool on);
    bool includeHidden() const { return m_includeHidden; }

    void setFolder(ShellFolder* folder);
    ShellFolder* folder() const { return m_folder; }

    const QList<ShellItem>& items() const { return m_items; }

    ShellItem itemAt(const QModelIndex& index) const;
    QModelIndex indexOf(const ShellItem& item, int column = 0) const;

    bool isParentFolder(const QModelIndex& index) const;

    void setItemSelected(const QModelIndex& index, bool selected);
    bool isItemSelected(const QModelIndex& index) const;

    void toggleItemSelected(const QModelIndex& index);

    QList<ShellItem> selectedItems() const;

    void selectAll();
    void unselectAll();
    void invertSelection();

    void calculateSize(const QModelIndex& index);
    void calculateSizeSelected();

    qint64 totalItemsSize() const;
    qint64 selectedItemsSize() const;

    int totalItemsCount() const;
    int selectedItemsCount() const;

    void compareWith(const QList<ShellItem>& items);

    void refresh();

    void setGotoItemName(const QString& name);
    const QModelIndex gotoItemIndex();

public: // overrides
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private slots:
    void extractNextIcon();

    void itemChanged(const ItemChange& change);
    void folderUpdated();

    void applyChanges();

private:
    int indexToRow(const QModelIndex& index) const;
    QModelIndex rowToIndex(int row, int column) const;

    void resort();

    bool shellItemLessThan(const ShellItem& item1, const ShellItem& item2);

    QStringList storeIndexes(const QModelIndexList& indexes);
    QModelIndexList restoreIndexes(const QStringList& names, const QModelIndexList& oldIndexes);

private:
    bool m_includeHidden{};

    ShellFolder* m_folder{};
    QList<ShellItem> m_items;

    bool m_hasParent{};

    int m_sortColumn{-1};
    Qt::SortOrder m_sortOrder{Qt::AscendingOrder};

    QList<int> m_extractQueue;

    QList<ItemChange> m_changes;
    bool m_pendingRefresh{};
    bool m_pendingUpdate{};

    QString m_gotoItemName;

    friend class ShellItemLessThan;
};
