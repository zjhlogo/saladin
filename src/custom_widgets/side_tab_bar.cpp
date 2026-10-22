// \file side_tab_bar.cpp
// \author zhangjianhua
// \date 2026/10/22

#include "side_tab_bar.h"

#include <QCursor>
#include <QPainter>

SideTabBar::SideTabBar(QWidget* parent)
    : QTabBar(parent)
{
    setShape(QTabBar::RoundedWest);
    setDrawBase(false);
    setExpanding(false);
    setMovable(false);
    setUsesScrollButtons(false);
    setFocusPolicy(Qt::NoFocus);

    // QTabBar tracks the tab under the cursor and repaints the affected rows
    // when the attribute is set; paintEvent() reads the position itself.
    setAttribute(Qt::WA_Hover);
}

/**
 * All rows share one width so the bar reads as a column: wide enough for the
 * longest label, but never below the minimum sidebar width. For a vertical
 * tab bar QTabBar uses the returned size as-is (width along the column,
 * height per row) and stacks the rows from top to bottom.
 */
QSize SideTabBar::tabSizeHint(int index) const
{
    const QFontMetrics metrics = fontMetrics();

    int width = K_MinRowWidth;
    for (int i = 0; i < count(); ++i)
    {
        const int contentWidth = iconSize().width() + K_IconTextSpace + metrics.horizontalAdvance(tabText(i));
        width = qMax(width, contentWidth + K_RowPadding * 2 + K_RowFrame * 2);
    }

    const int contentHeight = qMax(iconSize().height(), metrics.height());
    const int height = contentHeight + K_RowPadding * 2 + K_RowFrame * 2;

    return {width, height};
}

QRect SideTabBar::highlightRect(const QRect& rowRect) const
{
    QRect rect = rowRect;
    rect.adjust(K_RowFrame, K_RowFrame, -K_RowFrame, -K_RowFrame);
    return rect;
}

/**
 * Draws every tab as a row: the rounded highlight of the current (and
 * hovered) tab first, then icon and text at their usual orientation.
 */
void SideTabBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QColor textColor = palette().color(QPalette::WindowText);

    // Translucent overlays of the text color: the highlight follows both
    // color schemes without extra theme bookkeeping.
    QColor selectedColor = textColor;
    selectedColor.setAlpha(38);
    QColor hoverColor = textColor;
    hoverColor.setAlpha(20);

    const QFontMetrics metrics = fontMetrics();
    const QPoint cursorPosition = mapFromGlobal(QCursor::pos());

    for (int i = 0; i < count(); ++i)
    {
        const QRect row = highlightRect(tabRect(i));
        if (row.isEmpty()) continue;

        if (i == currentIndex())
            painter.setBrush(selectedColor);
        else if (i == tabAt(cursorPosition))
            painter.setBrush(hoverColor);
        else
            painter.setBrush(Qt::NoBrush);

        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(row, K_Radius, K_Radius);

        const bool enabled = isTabEnabled(i);

        QRect iconRect(QPoint(0, 0), iconSize());
        iconRect.moveTopLeft(QPoint(row.left() + K_RowPadding, row.top() + (row.height() - iconRect.height()) / 2));
        tabIcon(i).paint(&painter, iconRect, Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);

        const int textLeft = iconRect.right() + 1 + K_IconTextSpace;
        const int textWidth = row.right() - K_RowPadding - textLeft + 1;
        if (textWidth <= 0) continue;

        const QString text = metrics.elidedText(tabText(i), Qt::ElideRight, textWidth);
        painter.setPen(enabled ? textColor : palette().color(QPalette::Disabled, QPalette::WindowText));
        painter.drawText(QRect(textLeft, row.top(), textWidth, row.height()), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextHideMnemonic, text);
    }
}
