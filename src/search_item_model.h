// \file search_item_model.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_folder.h"
#include "shell/shell_item.h"
#include "shell/shell_pidl.h"

class SearchHelper;

class SearchItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Columns
    {
        Column_Name,
        Column_Size,
        Column_LastModified,
        Column_Attributes
    };

public:
    explicit SearchItemModel(QObject* parent);
    ~SearchItemModel();

public:
    void startSearch(ShellFolder* folder, const QString& pattern, const QString& text, Qt::CaseSensitivity cs);
    void abortSearch();

    bool isSearching() const;

    ShellItem itemAt(const QModelIndex& index) const;
    QString pathAt(const QModelIndex& index) const;
    ShellFolder* folderAt(const QModelIndex& index) const;

public: // overrides
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

signals:
    void folderEntered(const QString& path);

    void searchCompleted();

private slots:
    void scanNextFolder();

    void helperCompleted();

    void extractNextIcon();

private:
    void clear();

    bool scanFolder(const QString& prefix, ShellFolder* folder);

private:
    struct FoundItem
    {
        ShellItem m_item;
        ShellFolder* m_folder;
        QString m_prefix;
    };

    struct PendingFolder
    {
        ShellPidl m_pidl;
        QString m_prefix;
    };

    friend bool operator<(const PendingFolder& folder1, const PendingFolder& folder2);

private:
    QList<ShellFolder*> m_folders;

    QList<FoundItem> m_items;

    QList<PendingFolder> m_pendingFolders;

    QList<QRegularExpression> m_filters;

    QList<int> m_extractQueue;

    QString m_text;
    Qt::CaseSensitivity m_cs{Qt::CaseInsensitive};

    SearchHelper* m_helper{};

    QList<FoundItem> m_pendingItems;
};

class SearchProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SearchProxyModel(QObject* parent);
    ~SearchProxyModel();

protected: // overrides
    bool lessThan(const QModelIndex& index1, const QModelIndex& index2) const;
};
