// \file folder_item_view.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QTreeView>

class FolderItemView : public QTreeView
{
    Q_OBJECT
public:
    explicit FolderItemView(QWidget* parent);
    ~FolderItemView() override;

public:
    bool isEditing() const;

    QModelIndex movePageUp();
    QModelIndex movePageDown();

    void setAnchor(const QModelIndex& index);
    QModelIndex anchor() const;

    bool isDragging() const;
    void setDragging(bool dragging);

    void highlightDropItem(const QModelIndex& index);

    void checkAutoScroll(const QPoint& pos);

protected: // overrides
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

private:
    QPersistentModelIndex m_anchor;
};
