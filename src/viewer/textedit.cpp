// \file textedit.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "textedit.h"
#include "application.h"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEdit* parent)
        : QWidget(parent)
        , m_textEdit(parent)
    {
    }

public:
    QSize sizeHint() const override { return QSize(m_textEdit->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* e) override { m_textEdit->lineNumberAreaPaintEvent(e); }

private:
    TextEdit* m_textEdit;
};

TextEdit::TextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_lineNumbers(false)
{
    m_lineNumberArea = new LineNumberArea(this);
    m_lineNumberArea->hide();

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
    connect(this, SIGNAL(updateRequest(const QRect&, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));

    updateLineNumberAreaWidth();

    connect(g_application, SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

TextEdit::~TextEdit()
{
}

void TextEdit::setLineNumbers(bool numbers)
{
    if (m_lineNumbers != numbers)
    {
        m_lineNumbers = numbers;
        m_lineNumberArea->setVisible(numbers);
        updateLineNumberAreaWidth();
    }
}

int TextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        digits++;
    }

    return fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 4;
}

void TextEdit::updateLineNumberAreaWidth()
{
    setViewportMargins((m_lineNumbers ? lineNumberAreaWidth() : 0) + 2, 0, 0, 0);
}

void TextEdit::updateLineNumberArea(const QRect& rect, int dy)
{
    if (m_lineNumbers)
    {
        if (dy)
            m_lineNumberArea->scroll(0, dy);
        else
            m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

        if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth();
    }
}

void TextEdit::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect rect = contentsRect();
    m_lineNumberArea->setGeometry(QRect(rect.left(), rect.top(), lineNumberAreaWidth(), rect.height()));
}

void TextEdit::lineNumberAreaPaintEvent(QPaintEvent* e)
{
    // The theme in effect, not the raw "Theme" setting: with "Follow System"
    // the dark palette is used when Windows itself is dark.
    QColor background;
    QColor foreground;

    if (ThemeHelper::getAppTheme() == ThemeHelper::AppTheme::Dark)
    {
        background = QColor(16, 16, 16);
        foreground = QColor(128, 128, 128);
    }
    else
    {
        background = QColor(228, 228, 228);
        foreground = QColor(128, 128, 128);
    }

    QPainter painter(m_lineNumberArea);
    painter.fillRect(e->rect(), background);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    painter.setPen(foreground);

    while (block.isValid() && top <= e->rect().bottom())
    {
        if (block.isVisible() && bottom >= e->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, m_lineNumberArea->width() - 2, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        blockNumber++;
    }
}

void TextEdit::themeChanged()
{
    if (m_lineNumbers) m_lineNumberArea->update();
}
