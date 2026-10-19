// \file guide_dialog.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "bookmark.h"
#include "xmlui/client.h"

#include <QDialog>

class QTreeWidget;

class GuideDialog : public QDialog, public XmlUi::Client
{
    Q_OBJECT
public:
    GuideDialog(QWidget* parent);
    ~GuideDialog() override;

private slots:
    void goBack();
    void goForward();
    void goHome();

    void updateActions();

    void loadIcons();

private:
    QLabel* m_promptPixmap{};

    QTextBrowser* m_browser{};
};
