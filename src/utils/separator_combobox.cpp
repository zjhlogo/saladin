// \file separator_combobox.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "separator_combobox.h"

#include <QPainter>
#include <QStandardItem>

class SeparatorItemDelegate : public QAbstractItemDelegate
{
public:
    static constexpr int ITEM_TYPE_ROLE = Qt::UserRole + 1;

    enum ItemType
    {
        NormalItem,
        SeparatorItem,
        ParentItem,
        ChildItem
    };

public:
    SeparatorItemDelegate(QObject* parent, QAbstractItemDelegate* delegate)
        : QAbstractItemDelegate(parent)
        , m_delegate(delegate)
    {
    }

    ~SeparatorItemDelegate() override = default;

public: // overrides
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        switch (index.data(ITEM_TYPE_ROLE).toInt())
        {
        case SeparatorItem:
        {
            QStyleOptionViewItem noFocus = option;
            noFocus.state &= ~QStyle::State_HasFocus;

            m_delegate->paint(painter, noFocus, index);

            int y = (option.rect.top() + option.rect.bottom()) / 2;

            painter->setPen(option.palette.color(QPalette::Active, QPalette::Dark));
            painter->drawLine(option.rect.left(), y, option.rect.right(), y);
            break;
        }
        case ParentItem:
        {
            QStyleOptionViewItem parentOption = option;
            parentOption.state |= QStyle::State_Enabled;
            m_delegate->paint(painter, parentOption, index);
            break;
        }
        case ChildItem:
        {
            QStyleOptionViewItem childOption = option;
            int indent = option.fontMetrics.horizontalAdvance(QLatin1String("    "));
            childOption.rect.adjust(indent, 0, 0, 0);
            childOption.textElideMode = Qt::ElideNone;
            m_delegate->paint(painter, childOption, index);
            break;
        }
        default:
            m_delegate->paint(painter, option, index);
            break;
        }
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QSize size = m_delegate->sizeHint(option, index);

        int type = index.data(ITEM_TYPE_ROLE).toInt();
        if (type == SeparatorItem) size.setHeight(5);
        return size;
    }

private:
    QAbstractItemDelegate* m_delegate{};
};

SeparatorComboBox::SeparatorComboBox(QWidget* parent)
    : QComboBox(parent)
{
    QAbstractItemDelegate* delegate = view()->itemDelegate();
    view()->setItemDelegate(nullptr);

    setItemDelegate(new SeparatorItemDelegate(this, delegate));
}

SeparatorComboBox::~SeparatorComboBox() = default;

void SeparatorComboBox::addSeparator() const
{
    auto item = new QStandardItem(QString());
    item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    item->setData(SeparatorItemDelegate::SeparatorItem, SeparatorItemDelegate::ITEM_TYPE_ROLE);

    auto itemModel = dynamic_cast<QStandardItemModel*>(model());
    itemModel->appendRow(item);
}

void SeparatorComboBox::addParentItem(const QString& text) const
{
    auto item = new QStandardItem(text);
    item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    item->setData(SeparatorItemDelegate::ParentItem, SeparatorItemDelegate::ITEM_TYPE_ROLE);

    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);

    auto itemModel = dynamic_cast<QStandardItemModel*>(model());
    itemModel->appendRow(item);
}

void SeparatorComboBox::addChildItem(const QString& text, const QVariant& data /*= QVariant()*/) const
{
    auto item = new QStandardItem(text + QLatin1String("    "));
    item->setData(data, Qt::UserRole);
    item->setData(SeparatorItemDelegate::ChildItem, SeparatorItemDelegate::ITEM_TYPE_ROLE);

    auto itemModel = dynamic_cast<QStandardItemModel*>(model());
    itemModel->appendRow(item);
}
