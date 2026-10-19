// \file folder_item_model.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "folder_item_model.h"
#include "utils/icon_loader.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QTimer>
#include <algorithm>

// 兜底轮询间隔（毫秒）：仅在 shell 通知不可靠时启用。
static constexpr int PollInterval = 3000;       // 注册失败（虚拟文件夹、特殊位置等）
static constexpr int RemotePollInterval = 5000; // 网络位置：通知可能永远不来

// 单次校验耗时超过该值就放慢轮询，上限 MaxPollInterval。
static constexpr int SlowPollThreshold = 500;
static constexpr int MaxPollInterval = 60000;

class ShellItemLessThan
{
public:
    ShellItemLessThan(FolderItemModel* model)
        : m_model(model)
    {
    }

    bool operator()(const ShellItem& item1, const ShellItem& item2) { return m_model->shellItemLessThan(item1, item2); }

private:
    FolderItemModel* m_model;
};

FolderItemModel::FolderItemModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(PollInterval);

    connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(pollForChanges()));
}

FolderItemModel::~FolderItemModel()
{
}

void FolderItemModel::setIncludeHidden(bool on)
{
    m_includeHidden = on;
}

void FolderItemModel::setFolder(ShellFolder* folder)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    beginResetModel();

    delete m_folder;
    m_folder = folder;

    m_items = folderItems();

    connect(m_folder, SIGNAL(itemChanged(const ItemChange&)), this, SLOT(itemChanged(const ItemChange&)));
    connect(m_folder, SIGNAL(folderUpdated()), this, SLOT(folderUpdated()));

    // 本地磁盘上的 shell 通知是可靠的；注册失败（虚拟文件夹等）或目录不在本地
    // （网络位置的通知可能永远不来）时，退化为低频轮询兜底。
    if (m_folder->startWatching())
    {
        if (m_folder->isLocalFileSystemFolder())
        {
            m_pollTimer->stop();
        }
        else
        {
            m_pollTimer->setInterval(RemotePollInterval);
            m_pollTimer->start();
        }
    }
    else
    {
        m_pollTimer->setInterval(PollInterval);
        m_pollTimer->start();
    }

    m_hasParent = m_folder->hasParent();

    m_extractQueue.clear();
    m_changes.clear();
    m_pendingRefresh = false;

    endResetModel();

    QApplication::restoreOverrideCursor();
}

void FolderItemModel::refresh()
{
    if (!m_folder) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    setItems(folderItems());

    QApplication::restoreOverrideCursor();
}

void FolderItemModel::refreshIfChanged()
{
    if (!m_folder || !m_folder->isValid()) return;

    QList<ShellItem> items = folderItems();

    if (!itemsChanged(items)) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    setItems(items);

    QApplication::restoreOverrideCursor();
}

void FolderItemModel::pollForChanges()
{
    // 应用不在前台时不对网络位置做无谓的后台枚举；重新激活时
    // MainWindow 会再调用一次 refreshIfChanged() 补上这次校验。
    if (QApplication::applicationState() != Qt::ApplicationActive) return;

    QElapsedTimer timer;
    timer.start();

    refreshIfChanged();

    // 慢速位置（高延迟网络共享、超大目录）自动放慢轮询，避免周期性卡住界面；
    // 切换目录时会在 setFolder() 里恢复默认间隔。
    if (m_pollTimer->interval() > PollInterval && timer.elapsed() > SlowPollThreshold)
    {
        m_pollTimer->setInterval(qMin(m_pollTimer->interval() * 2, MaxPollInterval));
    }
}

QList<ShellItem> FolderItemModel::folderItems()
{
    ShellFolder::Flags flags;
    if (m_includeHidden) flags |= ShellFolder::IncludeHidden;

    QList<ShellItem> items = m_folder->listItems(flags);
    std::sort(items.begin(), items.end(), ShellItemLessThan(this));

    return items;
}

void FolderItemModel::setItems(const QList<ShellItem>& items)
{
    emit layoutAboutToBeChanged();

    QModelIndexList oldIndexes = persistentIndexList();
    QStringList names = storeIndexes(oldIndexes);

    m_items = items;

    QModelIndexList newIndexes = restoreIndexes(names, oldIndexes);
    changePersistentIndexList(oldIndexes, newIndexes);

    m_extractQueue.clear();
    m_changes.clear();
    m_pendingRefresh = false;
    m_gotoItemName.clear();

    emit layoutChanged();
}

bool FolderItemModel::itemsChanged(const QList<ShellItem>& items) const
{
    // 只比较视图真正显示的字段：名字唯一确定条目，其余字段决定显示内容。
    if (items.count() != m_items.count()) return true;

    for (int i = 0; i < items.count(); i++)
    {
        const ShellItem& item = items.at(i);
        const ShellItem& current = m_items.at(i);

        if (item.name() != current.name()) return true;
        if (item.size() != current.size()) return true;
        if (item.lastModified() != current.lastModified()) return true;
        if (item.attributes() != current.attributes()) return true;
    }

    return false;
}

void FolderItemModel::setGotoItemName(const QString& name)
{
    m_gotoItemName = name;
}

const QModelIndex FolderItemModel::gotoItemIndex()
{
    if (m_gotoItemName.isEmpty()) return QModelIndex();

    for (int j = 0; j < m_items.count(); j++)
    {
        if (m_items.at(j).name() == m_gotoItemName)
        {
            m_gotoItemName.clear();
            return rowToIndex(j, 0);
        }
    }

    return QModelIndex();
}

void FolderItemModel::itemChanged(const ItemChange& change)
{
    if (!m_pendingRefresh)
    {
        m_changes.append(change);

        if (!m_pendingUpdate)
        {
            QTimer::singleShot(100, this, SLOT(applyChanges()));
            m_pendingUpdate = true;
        }
    }
}

void FolderItemModel::folderUpdated()
{
    if (!m_pendingRefresh)
    {
        m_changes.clear();
        m_pendingRefresh = true;

        if (!m_pendingUpdate)
        {
            QTimer::singleShot(100, this, SLOT(applyChanges()));
            m_pendingUpdate = true;
        }
    }
}

void FolderItemModel::applyChanges()
{
    if (m_pendingRefresh) refresh();

    if (!m_changes.isEmpty())
    {
        emit layoutAboutToBeChanged();

        QModelIndexList oldIndexes = persistentIndexList();
        QStringList names = storeIndexes(oldIndexes);

        for (int i = 0; i < m_changes.count(); i++)
        {
            ItemChange& change = m_changes[i];
            bool found = false;

            for (int j = 0; j < m_items.count(); j++)
            {
                ShellItem& item = m_items[j];

                if (!(item == change.item1())) continue;

                found = true;

                switch (change.type())
                {
                case ItemChange::ItemAdded:
                case ItemChange::ItemUpdated:
                    // 变成隐藏而当前不显示隐藏文件，等同于被移除。
                    if (!m_includeHidden && change.item1().attributes().testFlag(ShellItem::Hidden))
                    {
                        removeItemAt(j, names);
                        break;
                    }

                    if (item.isSelected()) change.item1().setSelected(true);
                    item = change.item1();
                    break;

                case ItemChange::ItemRemoved:
                    removeItemAt(j, names);
                    break;

                case ItemChange::ItemRenamed:
                    // 重命名后变成隐藏时按移除处理（此时 item.name() 还是旧名字，映射有效）。
                    if (!m_includeHidden && change.item2().attributes().testFlag(ShellItem::Hidden))
                    {
                        removeItemAt(j, names);
                        break;
                    }

                    for (int k = 0; k < names.count(); k++)
                    {
                        QString& name = names[k];
                        if (name == item.name()) name = change.item2().name();
                    }
                    if (item.isSelected()) change.item2().setSelected(true);
                    item = change.item2();
                    break;
                }

                break;
            }

            if (!found && (change.type() == ItemChange::ItemAdded || change.type() == ItemChange::ItemUpdated))
            {
                // 由隐藏变为可见，或者通知先于目录枚举到达。
                if (!m_includeHidden && change.item1().attributes().testFlag(ShellItem::Hidden)) continue;

                m_items.append(change.item1());
            }
        }

        std::sort(m_items.begin(), m_items.end(), ShellItemLessThan(this));

        QModelIndexList newIndexes = restoreIndexes(names, oldIndexes);
        changePersistentIndexList(oldIndexes, newIndexes);

        m_extractQueue.clear();
        m_changes.clear();

        emit layoutChanged();
    }

    m_pendingUpdate = false;
}

void FolderItemModel::removeItemAt(int row, QStringList& names)
{
    // 被移除条目的持久化索引改挂到相邻条目上，避免整行选中丢失。
    for (int k = 0; k < names.count(); k++)
    {
        QString& name = names[k];

        if (name != m_items.at(row).name()) continue;

        if (row < m_items.count() - 1)
            name = m_items.at(row + 1).name();
        else if (row > 0)
            name = m_items.at(row - 1).name();
        else
            name.clear();
    }

    m_items.removeAt(row);
}

QStringList FolderItemModel::storeIndexes(const QModelIndexList& indexes)
{
    QStringList names;

    foreach (QModelIndex index, indexes)
        names.append(itemAt(index).name());

    return names;
}

QModelIndexList FolderItemModel::restoreIndexes(const QStringList& names, const QModelIndexList& oldIndexes)
{
    QModelIndexList newIndexes;

    for (int i = 0; i < names.count(); i++)
    {
        QModelIndex index;
        for (int j = 0; j < m_items.count(); j++)
        {
            if (m_items.at(j).name() == names.at(i))
            {
                index = rowToIndex(j, oldIndexes.at(i).column());
                break;
            }
        }
        if (!index.isValid() && m_hasParent) index = createIndex(0, oldIndexes.at(i).column());
        newIndexes.append(index);
    }

    return newIndexes;
}

void FolderItemModel::extractNextIcon()
{
    if (m_extractQueue.isEmpty()) return;

    int row = m_extractQueue.takeFirst();

    m_folder->extractIcon(m_items[row]);

    QModelIndex index = rowToIndex(row, 0);
    emit dataChanged(index, index);

    if (!m_extractQueue.isEmpty()) QTimer::singleShot(0, this, SLOT(extractNextIcon()));
}

int FolderItemModel::indexToRow(const QModelIndex& index) const
{
    if (!index.isValid()) return -1;

    int row = index.row() - (m_hasParent ? 1 : 0);
    if (row < 0 || row >= m_items.count()) return -1;

    return row;
}

QModelIndex FolderItemModel::rowToIndex(int row, int column) const
{
    return createIndex(row + (m_hasParent ? 1 : 0), column);
}

ShellItem FolderItemModel::itemAt(const QModelIndex& index) const
{
    int row = indexToRow(index);
    if (row < 0) return ShellItem();
    return m_items.at(row);
}

QModelIndex FolderItemModel::indexOf(const ShellItem& item, int column /*= 0*/) const
{
    for (int i = 0; i < m_items.count(); i++)
    {
        if (m_items.at(i) == item) return rowToIndex(i, column);
    }
    return QModelIndex();
}

bool FolderItemModel::isParentFolder(const QModelIndex& index) const
{
    return m_hasParent && index.isValid() && index.row() == 0;
}

void FolderItemModel::setItemSelected(const QModelIndex& index, bool selected)
{
    int row = indexToRow(index);
    if (row >= 0)
    {
        m_items[row].setSelected(selected);
        emit dataChanged(rowToIndex(row, 0), rowToIndex(row, Num_Column - 1));
    }
}

bool FolderItemModel::isItemSelected(const QModelIndex& index) const
{
    int row = indexToRow(index);
    if (row < 0) return false;
    return m_items.at(row).isSelected();
}

void FolderItemModel::toggleItemSelected(const QModelIndex& index)
{
    int row = indexToRow(index);
    if (row >= 0)
    {
        m_items[row].setSelected(!m_items.at(row).isSelected());
        emit dataChanged(rowToIndex(row, 0), rowToIndex(row, Num_Column - 1));
    }
}

QList<ShellItem> FolderItemModel::selectedItems() const
{
    QList<ShellItem> result;

    foreach (ShellItem item, m_items)
    {
        if (item.isSelected()) result.append(item);
    }

    return result;
}

void FolderItemModel::selectAll()
{
    for (int i = 0; i < m_items.count(); i++)
        m_items[i].setSelected(true);
    emit dataChanged(rowToIndex(0, 0), rowToIndex(m_items.count() - 1, Num_Column - 1));
}

void FolderItemModel::unselectAll()
{
    for (int i = 0; i < m_items.count(); i++)
        m_items[i].setSelected(false);
    emit dataChanged(rowToIndex(0, 0), rowToIndex(m_items.count() - 1, Num_Column - 1));
}

void FolderItemModel::invertSelection()
{
    for (int i = 0; i < m_items.count(); i++)
        m_items[i].setSelected(!m_items[i].isSelected());
    emit dataChanged(rowToIndex(0, 0), rowToIndex(m_items.count() - 1, Num_Column - 1));
}

void FolderItemModel::calculateSize(const QModelIndex& index)
{
    int row = indexToRow(index);
    if (row >= 0)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        if (m_folder->calculateSize(m_items[row]))
        {
            if (m_sortColumn == Column_Size)
                resort();
            else
                emit dataChanged(rowToIndex(row, 0), rowToIndex(row, Num_Column - 1));
        }

        QApplication::restoreOverrideCursor();
    }
}

void FolderItemModel::calculateSizeSelected()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (int i = 0; i < m_items.count(); i++)
    {
        if (m_items.at(i).isSelected()) m_folder->calculateSize(m_items[i]);
    }

    if (m_sortColumn == Column_Size)
        resort();
    else
        emit dataChanged(rowToIndex(0, 0), rowToIndex(m_items.count() - 1, Num_Column - 1));

    QApplication::restoreOverrideCursor();
}

qint64 FolderItemModel::totalItemsSize() const
{
    qint64 size = 0;

    foreach (ShellItem item, m_items)
        size += item.size();

    return size;
}

qint64 FolderItemModel::selectedItemsSize() const
{
    qint64 size = 0;

    foreach (ShellItem item, m_items)
    {
        if (item.isSelected()) size += item.size();
    }

    return size;
}

int FolderItemModel::totalItemsCount() const
{
    return m_items.count();
}

int FolderItemModel::selectedItemsCount() const
{
    int count = 0;

    foreach (ShellItem item, m_items)
    {
        if (item.isSelected()) count++;
    }

    return count;
}

void FolderItemModel::compareWith(const QList<ShellItem>& items)
{
    for (int i = 0; i < m_items.count(); i++)
    {
        bool match = false;
        for (int j = 0; j < items.count(); j++)
        {
            if (m_items.at(i) == items.at(j))
            {
                if (!m_items.at(i).state().testFlag(ShellItem::HasProperties) || !items.at(j).state().testFlag(ShellItem::HasProperties)
                    || m_items.at(i).lastModified() <= items.at(j).lastModified())
                {
                    match = true;
                }
                break;
            }
        }
        m_items[i].setSelected(!match);
    }

    emit dataChanged(rowToIndex(0, 0), rowToIndex(m_items.count() - 1, Num_Column - 1));
}

int FolderItemModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid()) return (int)Num_Column;
    return 0;
}

QVariant FolderItemModel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case Column_Name:
            return tr("Name");
        case Column_Ext:
            return tr("Ext");
        case Column_Size:
            return tr("Size");
        case Column_LastModified:
            return tr("Date");
        case Column_Attributes:
            return tr("Attributes");
        }
    }
    return QVariant();
}

int FolderItemModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid()) return m_items.count() + (m_hasParent ? 1 : 0);
    return 0;
}

QModelIndex FolderItemModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid()) return createIndex(row, column);
    return QModelIndex();
}

QModelIndex FolderItemModel::parent(const QModelIndex& /*index*/) const
{
    return QModelIndex();
}

static QString formatAttributes(ShellItem::Attributes attributes)
{
    char result[8];
    result[0] = attributes.testFlag(ShellItem::Directory) ? 'd' : '-';
    result[1] = attributes.testFlag(ShellItem::ReadOnly) ? 'r' : '-';
    result[2] = attributes.testFlag(ShellItem::Archive) ? 'a' : '-';
    result[3] = attributes.testFlag(ShellItem::Hidden) ? 'h' : '-';
    result[4] = attributes.testFlag(ShellItem::System) ? 's' : '-';
    result[5] = attributes.testFlag(ShellItem::Compressed) ? 'c' : '-';
    result[6] = attributes.testFlag(ShellItem::Encrypted) ? 'e' : '-';
    result[7] = 0;
    return result;
}

QVariant FolderItemModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (isParentFolder(index))
        {
            if (index.column() == Column_Name) return "..";
            if (index.column() == Column_Size) return tr("<DIR>");
            return QVariant();
        }

        int row = indexToRow(index);
        if (row >= 0)
        {
            const ShellItem& item = m_items.at(row);

            switch (index.column())
            {
            case Column_Name:
                return item.name();
            case Column_Ext:
                return item.ext();
            case Column_Size:
                if (item.attributes().testFlag(ShellItem::Directory))
                {
                    if (item.attributes().testFlag(ShellItem::ReparsePoint)) return tr("<LNK>");
                    if (!item.state().testFlag(ShellItem::HasCalculatedSize)) return tr("<DIR>");
                }
                if (item.state().testFlag(ShellItem::HasProperties)) return item.humanReadableSize() + ' ';
                break;
            case Column_LastModified:
                if (item.state().testFlag(ShellItem::HasProperties)) return item.lastModified().toString("yyyy-MM-dd hh:mm");
                break;
            case Column_Attributes:
                if (item.state().testFlag(ShellItem::HasProperties)) return formatAttributes(item.attributes());
                break;
            }
        }

        return QVariant();
    }

    if (role == Qt::DecorationRole && index.column() == Column_Name)
    {
        if (isParentFolder(index)) return IconLoader::pixmap("arrow-up");

        int row = indexToRow(index);
        if (row >= 0)
        {
            if (!(m_items.at(row).state().testFlag(ShellItem::HasExtractedIcon)) && !m_extractQueue.contains(row))
            {
                auto that = const_cast<FolderItemModel*>(this);
                if (m_extractQueue.isEmpty()) QTimer::singleShot(0, that, SLOT(extractNextIcon()));
                that->m_extractQueue.append(row);
            }

            return m_items.at(row).icon();
        }
    }

    if (role == Qt::ToolTipRole && index.column() == Column_Name)
    {
        int row = indexToRow(index);
        if (row >= 0)
        {
            if (!m_items.at(row).attributes().testFlag(ShellItem::Directory)) return m_folder->toolTip(m_items.at(row));
        }
    }

    if (role == Qt::TextAlignmentRole && index.column() == Column_Size)
    {
        int row = indexToRow(index);
        if (row >= 0)
        {
            const ShellItem& item = m_items.at(row);
            if (!item.attributes().testFlag(ShellItem::Directory) || item.state().testFlag(ShellItem::HasCalculatedSize))
                return (int)(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    if (role == Qt::ForegroundRole)
    {
        int row = indexToRow(index);
        if (row >= 0)
        {
            const ShellItem& item = m_items.at(row);

            if (item.isSelected()) return QColor(255, 0, 0);

            if (item.attributes().testFlag(ShellItem::Hidden)) return QApplication::palette().color(QPalette::Disabled, QPalette::Text);

            if (item.attributes().testFlag(ShellItem::Compressed)) return QApplication::palette().color(QPalette::Link);
        }
    }

    return QVariant();
}

bool FolderItemModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::EditRole)
    {
        int row = indexToRow(index);
        if (row >= 0 && index.column() == 0)
        {
            if (m_folder->setItemName(m_items[row], value.toString()))
            {
                resort();
                return true;
            }
        }
    }

    return false;
}

Qt::ItemFlags FolderItemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::ItemFlags();

    Qt::ItemFlags result = Qt::ItemIsEnabled;

    int row = indexToRow(index);
    if (row >= 0 && index.column() == 0)
    {
        if (m_items.at(row).attributes().testFlag(ShellItem::CanRename)) result |= Qt::ItemIsEditable;
    }

    return result;
}

void FolderItemModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
    if (m_sortColumn == column && m_sortOrder == order) return;

    m_sortColumn = column;
    m_sortOrder = order;

    resort();
}

void FolderItemModel::resort()
{
    emit layoutAboutToBeChanged();

    QModelIndexList oldIndexes = persistentIndexList();
    QStringList names = storeIndexes(oldIndexes);

    std::sort(m_items.begin(), m_items.end(), ShellItemLessThan(this));

    QModelIndexList newIndexes = restoreIndexes(names, oldIndexes);
    changePersistentIndexList(oldIndexes, newIndexes);

    m_extractQueue.clear();

    emit layoutChanged();
}

bool FolderItemModel::shellItemLessThan(const ShellItem& item1, const ShellItem& item2)
{
    bool dir1 = (item1.attributes().testFlag(ShellItem::Directory));
    bool dir2 = (item2.attributes().testFlag(ShellItem::Directory));

    if (dir1 && !dir2) return true;
    if (!dir1 && dir2) return false;

    int result = 0;

    switch (m_sortColumn)
    {
    case Column_Size:
        if (item1.size() < item2.size())
            result = -1;
        else if (item2.size() < item1.size())
            result = 1;
        break;
    case Column_Ext:
        if (item1.ext() < item2.ext())
            result = -1;
        else if (item2.ext() < item1.ext())
            result = 1;
        break;
    case Column_LastModified:
        if (item1.lastModified() < item2.lastModified())
            result = -1;
        else if (item2.lastModified() < item1.lastModified())
            result = 1;
        break;
    case Column_Attributes:
        result = QString::localeAwareCompare(formatAttributes(item1.attributes()), formatAttributes(item2.attributes()));
        break;
    }

    if (result == 0) result = StrCmpLogicalW((LPCWSTR)item1.name().utf16(), (LPCWSTR)item2.name().utf16());

    if (m_sortOrder == Qt::AscendingOrder)
        return result < 0;
    else
        return result > 0;
}
