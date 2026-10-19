// \file text_edit.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

class LineNumberArea;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    TextEdit(QWidget* parent);
    ~TextEdit() override;

public:
    void setLineNumbers(bool numbers);
    bool lineNumbers() const { return m_lineNumbers; }

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect& rect, int dy);

    void themeChanged();

private:
    void lineNumberAreaPaintEvent(QPaintEvent* e);
    int lineNumberAreaWidth();

private:
    LineNumberArea* m_lineNumberArea{};
    bool m_lineNumbers;

    friend class LineNumberArea;
};
