// \file side_tab_bar.h
// \author zhangjianhua
// \date 2026/10/22

#pragma once

#include <QTabBar>

/**
 * Vertical navigation tab bar.
 *
 * Every tab is drawn as a horizontal row with the icon on the left and the
 * text next to it, stacked from top to bottom along the left side of the
 * dialog. The selected tab is highlighted with a rounded background and
 * hovered tabs with a weaker version of the same highlight; the text keeps
 * its normal orientation.
 *
 * QTabBar is still responsible for the layout (RoundedWest stacks the rows)
 * and for the mouse handling, only the painting is replaced.
 *
 * The highlight colors are derived from the widget palette, so the bar works
 * with the light and the dark Qt-Material theme alike.
 */
class SideTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit SideTabBar(QWidget* parent = nullptr);

protected: // overrides
    QSize tabSizeHint(int index) const override;
    void paintEvent(QPaintEvent* event) override;

private:
    QRect highlightRect(const QRect& rowRect) const;

private:
    // Row geometry shared by tabSizeHint() and paintEvent().
    static constexpr int K_RowFrame = 3;      // empty frame between the rows
    static constexpr int K_RowPadding = 8;    // padding of the row contents
    static constexpr int K_IconTextSpace = 8; // gap between icon and text
    static constexpr int K_MinRowWidth = 124; // width of the sidebar column
    static constexpr int K_Radius = 4;        // corner radius of the highlight
};
