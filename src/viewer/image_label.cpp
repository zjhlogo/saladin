// \file image_label.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "image_label.h"

ImageLabel::ImageLabel(QWidget* parent)
    : QWidget(parent)
    , m_zoom(1.0)
    , m_black(false)
{
    setAttribute(Qt::WA_OpaquePaintEvent);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

ImageLabel::~ImageLabel()
{
}

void ImageLabel::setImage(const QImage& image)
{
    m_image = image;
    updateGeometry();
    update();
}

void ImageLabel::setZoom(double zoom)
{
    m_zoom = zoom;
    if (zoom < 0.0)
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    else
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    updateGeometry();
    update();

    emit zoomChanged();
}

void ImageLabel::setBlackBackground(bool black)
{
    m_black = black;
    update();
}

double ImageLabel::actualZoom() const
{
    if (m_zoom < 0.0)
    {
        double sx = double(width()) / m_image.width();
        double sy = double(height()) / m_image.height();
        return qMin(qMin(sx, sy), 1.0);
    }
    else
    {
        return m_zoom;
    }
}

QSize ImageLabel::sizeHint() const
{
    if (m_zoom < 0.0)
        return QSize();
    else
        return m_zoom * m_image.size();
}

void ImageLabel::paintEvent(QPaintEvent* /*e*/)
{
    QPainter painter(this);

    painter.fillRect(rect(), m_black ? Qt::black : Qt::white);

    if (m_image.isNull()) return;

    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    double zoom = actualZoom();

    QSize scaled = zoom * m_image.size();

    QRect target;
    target.setTop((height() - scaled.height()) / 2);
    target.setLeft((width() - scaled.width()) / 2);
    target.setSize(scaled);

    painter.drawImage(target, m_image);
}

void ImageLabel::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->angleDelta().y() > 0)
            emit zoomIn();
        else
            emit zoomOut();
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void ImageLabel::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);

    if (m_zoom < 0.0) emit zoomChanged();
}
