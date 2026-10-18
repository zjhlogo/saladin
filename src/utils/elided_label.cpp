// \file elided_label.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "elided_label.h"

#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

ElidedLabel::ElidedLabel(QWidget* parent)
    : QLabel(parent)
    , m_lastWidth(0)
{
    setMinimumWidth(fontMetrics().horizontalAdvance(QLatin1String("...")));
}

ElidedLabel::~ElidedLabel()
{
}

void ElidedLabel::paintEvent(QPaintEvent* /*e*/)
{
    QPainter painter(this);
    drawFrame(&painter);

    QRect cr = contentsRect();
    cr.adjust(margin(), margin(), -margin(), -margin());

    QString fullText = text();

    if (fullText != m_lastText || cr.width() != m_lastWidth)
    {
        m_elidedText = fontMetrics().elidedText(fullText, Qt::ElideRight, cr.width());
        m_lastText = fullText;
        m_lastWidth = cr.width();
    }

    QStyleOption opt;
    opt.initFrom(this);

    style()->drawItemText(&painter, cr, alignment(), opt.palette, isEnabled(), m_elidedText, foregroundRole());
}
