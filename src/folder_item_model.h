// \file folder_item_model.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_folder.h"
#include "shell/shell_item.h"

#include <QMainWindow>

class QTimer;

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

    // 重新枚举目录，只有内容确实变化时才更新视图（用于通知丢失后的兜底校验）。
    void refreshIfChanged();

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

signals:
    // 当前目录已被外部删除/移动，面板需要换到别的目录。
    void folderLost();

private slots:
    void extractNextIcon();

    void itemChanged(const ItemChange& change);
    void folderUpdated();

    void applyChanges();

    void pollForChanges();

private:
    int indexToRow(const QModelIndex& index) const;
    QModelIndex rowToIndex(int row, int column) const;

    void resort();

    QList<ShellItem> folderItems();

    void setItems(const QList<ShellItem>& items);

    bool itemsChanged(const QList<ShellItem>& items) const;

    void removeItemAt(int row, QStringList& names);

    void reportFolderLost();

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

    // shell 通知为主，轮询只做兜底：补上被合并/丢失的通知，以及当前目录
    // 被外部整个删除（shell 未必发通知）的情况。
    QTimer* m_pollTimer{};

    // 已经报告过“目录不存在”，等面板切换目录，避免重复导航。
    bool m_folderLost{};

    QString m_gotoItemName;

    friend class ShellItemLessThan;
};
