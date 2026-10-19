// \file viewer_window.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "viewer/view.h"
#include "xmlui/client.h"

#include <QMainWindow>

class ViewerWindow : public QMainWindow, public XmlUi::Client
{
    Q_OBJECT
public:
    ViewerWindow();
    ~ViewerWindow() override;

public:
    void setView(View* view);

    void enableNavigation(bool isFirst, bool isLast);

protected: // overrides
    void showEvent(QShowEvent* e) override;

private slots:
    void previous();
    void next();

    void reload();

    void switchToText();
    void switchToBinary();
    void switchToImage();

    void fullScreen();

    void statusChanged(const QString& status);

    void loadIcons();

private:
    void initializeGeometry();
    void storeGeometry(bool offset);

private:
    View* m_view;

    QLabel* m_statusLabel;

    bool m_restoreMaximized;
};
