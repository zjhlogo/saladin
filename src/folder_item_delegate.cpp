// \file folder_item_delegate.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "folder_item_delegate.h"
#include "application.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>

FolderItemDelegate::FolderItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

FolderItemDelegate::~FolderItemDelegate()
{
}

void FolderItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QBrush foreground = index.data(Qt::ForegroundRole).value<QBrush>();

    // The application-wide stylesheet sets a text color on "QTreeView::item",
    // and QStyleSheetStyle applies it over the palette of every item before
    // the base style draws the text, so the per-item colors the model returns
    // in Qt::ForegroundRole (red for items marked with the space key, dimmed
    // hidden files, colored compressed files) never reach the screen. Draw
    // such rows explicitly. View-selected rows keep the regular painting: the
    // stylesheet gives the selection its own colors, which must win.
    if ((option.state & QStyle::State_Selected) || foreground.style() == Qt::NoBrush)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    paintWithForeground(painter, option, index, foreground);
}

void FolderItemDelegate::paintWithForeground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index,
                                             const QBrush& foreground) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget* widget = option.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();

    // Everything except the text: background, selection state and icon.
    QStyleOptionViewItem withoutText = opt;
    withoutText.text.clear();
    style->drawControl(QStyle::CE_ItemViewItem, &withoutText, painter, widget);

    // The text with the color of the model, laid out the way QCommonStyle
    // lays it out: the text subelement rect, inset by the focus-frame margin
    // on both sides and elided with the mode configured on the view.
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option, widget) + 1;
    const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget).adjusted(textMargin, 0, -textMargin, 0);
    const QString text = opt.fontMetrics.elidedText(opt.text, opt.textElideMode, textRect.width());

    painter->save();
    painter->setFont(opt.font);
    painter->setPen(foreground.color());
    painter->drawText(textRect, QStyle::visualAlignment(opt.direction, opt.displayAlignment), text);
    painter->restore();
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
