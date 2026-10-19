// \file folder_item_delegate.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "shell/shell_folder.h"
#include "shell/shell_item.h"

#include <QBrush>
#include <QStyledItemDelegate>

class FolderItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FolderItemDelegate(QObject* parent);
    ~FolderItemDelegate() override;

public: // overrides
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    bool eventFilter(QObject* object, QEvent* e) override;

private:
    void paintWithForeground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index,
                             const QBrush& foreground) const;
};
