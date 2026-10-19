// \file image_view.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "viewer/view.h"

class ImageLabel;
class ImageLoader;

class ImageView : public View
{
    Q_OBJECT
public:
    ImageView(QObject* parent, QWidget* parentWidget);
    ~ImageView() override;

public: // overrides
    Type type() const override;

    void load() override;

    void storeSettings() override;

private slots:
    void updateActions();

    void copy();

    void zoomFit();

    void zoomIn();
    void zoomOut();
    void zoomOriginal();

    void rotateLeft();
    void rotateRight();

    void blackBackground();

    void viewInformation();

    void contextMenuRequested(const QPoint& pos);

    void loadImage();

    void updateStatus();

    void loadIcons();

private:
    void initializeSettings();

    void zoom(double factor);
    void adjustScrollBar(QScrollBar* scrollBar, double factor);

private:
    QScrollArea* m_scroll;

    ImageLabel* m_label;

    ImageLoader* m_loader;
};
