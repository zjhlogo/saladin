// \file folder_item_delegate.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "folder_item_delegate.h"
#include "application.h"

FolderItemDelegate::FolderItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

FolderItemDelegate::~FolderItemDelegate()
{
}

void FolderItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    QPalette palette = Application::palette();
    palette.setColor(QPalette::Text, palette.color(QPalette::WindowText));
    editor->setPalette(palette);

    editor->setGeometry(option.rect.adjusted(21, 0, 0, 0));
}

bool FolderItemDelegate::eventFilter(QObject* object, QEvent* e)
{
    if (e->type() == QEvent::KeyPress)
    {
        auto keyEvent = static_cast<QKeyEvent*>(e);
        switch (keyEvent->key())
        {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            return false;
        }
    }

    return QStyledItemDelegate::eventFilter(object, e);
}
