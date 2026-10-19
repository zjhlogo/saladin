// \file settings_dialog.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include "shell/shell_pidl.h"

class SeparatorComboBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent);
    ~SettingsDialog() override;

public: // overrides
    void accept() override;

private slots:
    bool apply();

    void browseLeftPane();
    void browseRightPane();

    void browseViewer();
    void browseEditor();
    void browseConsole();
    void browseDiff();

    void showGeneralTab() const;
    void showToolsTab() const;

    void loadIcons() const;

private:
    void browsePane(QLineEdit* edit, int index);
    void browseTool(QLineEdit* edit);

    void loadLanguages() const;

    QToolButton* browseButton(QWidget* parent, const char* slot) const;

private:
    QLabel* m_promptPixmap{};

    QStackedWidget* m_stackedWidget{};

    QAction* m_generalAction{};
    QAction* m_toolsAction{};

    QToolButton* m_leftPaneButton{};
    QToolButton* m_rightPaneButton{};
    QToolButton* m_viewerButton{};
    QToolButton* m_editorButton{};
    QToolButton* m_consoleButton{};
    QToolButton* m_diffButton{};

    SeparatorComboBox* m_languageComboBox{};

    QLineEdit* m_leftPaneEdit{};
    QLineEdit* m_rightPaneEdit{};
    QCheckBox* m_rememberCheckBox{};

    QComboBox* m_themeComboBox{};

    QFontComboBox* m_binaryFontComboBox{};
    QSpinBox* m_binaryFontSpinBox{};
    QFontComboBox* m_textFontComboBox{};
    QSpinBox* m_textFontSpinBox{};

    QCheckBox* m_confirmDndCheckBox{};

    QCheckBox* m_updateCheckBox{};

    QCheckBox* m_internalViewerCheckBox{};

    QLineEdit* m_viewerEdit{};
    QLineEdit* m_editorEdit{};
    QLineEdit* m_consoleEdit{};
    QLineEdit* m_diffEdit{};

    ShellPidl m_pidls[2]{};
};
