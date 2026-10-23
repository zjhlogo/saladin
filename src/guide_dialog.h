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

public: // overrides
    void changeEvent(QEvent* e) override;

private slots:
    void goBack();
    void goForward();
    void goHome();

    void updateActions();

    // Re-applies the theme colors of the guide document (the linked light
    // stylesheet is overridden by style-dark.css on the dark theme).
    void updateGuideStyle();

    void loadIcons();

private:
    void retranslateUi();

private:
    QPushButton* m_okButton{};

    QTextBrowser* m_browser{};
};
