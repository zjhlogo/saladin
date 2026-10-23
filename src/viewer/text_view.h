// \file text_view.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "viewer/view.h"

class TextEdit;
class FindBar;
class TextLoader;

class TextView : public View
{
    Q_OBJECT
public:
    TextView(QObject* parent, QWidget* parentWidget);
    ~TextView() override;

public: // overrides
    Type type() const override;

    void load() override;

    void storeSettings() override;

    void retranslateUi() override;

    void setFullScreen(bool on) override;

    bool eventFilter(QObject* obj, QEvent* e) override;

private slots:
    void updateActions();

    void copy();
    void selectAll();

    void find();
    void findNext();
    void findPrevious();

    void findText(const QString& text);

    void toggleLineNumbers();
    void goToLine();

    void toggleWordWrap();
    void setEncoding(const QString& format);

    void selectEncoding();

    void editFile();

    void contextMenuRequested(const QPoint& pos);

    void loadNextBlock();

    void settingsChanged();

    void loadIcons();

private:
    void initializeSettings();

    QMenu* createEncodingMenu();

    void findText(const QString& text, int from, QTextDocument::FindFlags flags);

private:
    TextEdit* m_edit;

    TextLoader* m_loader;

    qint64 m_length;

    QString m_encoding;

    FindBar* m_findBar;

    bool m_isFindEnabled;

    int m_currentLine;
};
