// \file image_label.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

class ImageLabel : public QWidget
{
    Q_OBJECT
public:
    ImageLabel(QWidget* parent);
    ~ImageLabel();

public:
    void setImage(const QImage& image);
    const QImage& image() const { return m_image; }

    void setZoom(double zoom);
    double zoom() const { return m_zoom; }

    void setBlackBackground(bool black);
    bool isBlackBackground() const { return m_black; }

    double actualZoom() const;

public: // overrides
    QSize sizeHint() const;

protected: // overrides
    void paintEvent(QPaintEvent* e);

    void wheelEvent(QWheelEvent* e);

    void resizeEvent(QResizeEvent* e);

signals:
    void zoomIn();
    void zoomOut();

    void zoomChanged();

private:
    QImage m_image;

    double m_zoom;

    bool m_black;
};
