// \file folder_item_delegate.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "shell/shell_folder.h"
#include "shell/shell_item.h"

#include <QStyledItemDelegate>

class FolderItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FolderItemDelegate(QObject* parent);
    ~FolderItemDelegate() override;

public: // overrides
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    bool eventFilter(QObject* object, QEvent* e) override;
};
