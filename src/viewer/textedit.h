/**************************************************************************
 * This file is part of the Saladin program
 * Copyright (C) 2011-2017 Michał Męciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

class LineNumberArea;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    TextEdit(QWidget* parent);
    ~TextEdit();

public:
    void setLineNumbers(bool numbers);
    bool lineNumbers() const { return m_lineNumbers; }

protected:
    void resizeEvent(QResizeEvent* e);

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect& rect, int dy);

    void themeChanged();

private:
    void lineNumberAreaPaintEvent(QPaintEvent* e);
    int lineNumberAreaWidth();

private:
    bool m_lineNumbers;

    LineNumberArea* m_lineNumberArea;

    friend class LineNumberArea;
};

#endif
