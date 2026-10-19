// \file folder_item_view.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "folder_item_view.h"

#include <private/qtreeview_p.h>

FolderItemView::FolderItemView(QWidget* parent)
    : QTreeView(parent)
{
}

FolderItemView::~FolderItemView()
{
}

bool FolderItemView::isEditing() const
{
    return state() == EditingState;
}

QModelIndex FolderItemView::movePageUp()
{
    return moveCursor(QAbstractItemView::MovePageUp, Qt::NoModifier);
}

QModelIndex FolderItemView::movePageDown()
{
    return moveCursor(QAbstractItemView::MovePageDown, Qt::NoModifier);
}

void FolderItemView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QTreeView::drawRow(painter, option, index);

    if (hasFocus() && index.row() == currentIndex().row() && state() != QAbstractItemView::EditingState)
    {
        QRect rect(-header()->offset(), option.rect.top(), header()->length(), option.rect.height());
        rect = style()->visualRect(layoutDirection(), viewport()->rect(), rect);

        QPen oldPen = painter->pen();
        painter->setPen(palette().color(QPalette::Highlight));
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->setPen(oldPen);
    }
}

void FolderItemView::setAnchor(const QModelIndex& index)
{
    m_anchor = index;
}

QModelIndex FolderItemView::anchor() const
{
    return m_anchor;
}

void FolderItemView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QTreeView::currentChanged(current, previous);
    m_anchor = QPersistentModelIndex();
}

bool FolderItemView::isDragging() const
{
    return state() == DraggingState;
}

void FolderItemView::setDragging(bool dragging)
{
    if (dragging)
    {
        setState(DraggingState);
    }
    else
    {
        stopAutoScroll();
        setState(NoState);
        viewport()->update();
    }
}

void FolderItemView::highlightDropItem(const QModelIndex& index)
{
    auto d = static_cast<QTreeViewPrivate*>(d_ptr.data());

    if (index.isValid())
    {
        QRect rect = visualRect(index.sibling(index.row(), 0));
        QRect rect2 = visualRect(index.sibling(index.row(), model()->columnCount() - 1));
        rect.setRight(rect2.right());
        d->dropIndicatorRect = rect.adjusted(0, 0, -1, -1);
    }
    else
    {
        d->dropIndicatorRect = QRect();
    }

    viewport()->update();
}

void FolderItemView::checkAutoScroll(const QPoint& pos)
{
    auto d = static_cast<QTreeViewPrivate*>(d_ptr.data());

    if (!d->autoScroll) return;

    QRect area = viewport()->visibleRegion().boundingRect();

    if ((pos.y() - area.top() < d->autoScrollMargin) || (area.bottom() - pos.y() < d->autoScrollMargin) || (pos.x() - area.left() < d->autoScrollMargin)
        || (area.right() - pos.x() < d->autoScrollMargin))
    {
        startAutoScroll();
    }
}
