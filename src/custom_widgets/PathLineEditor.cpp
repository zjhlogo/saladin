// \file PathLineEditor.cpp
// \author zhangjianhua
// \date 2026/10/16

#include "PathLineEditor.h"

PathLineEditor::PathLineEditor(QWidget* parent)
    : QLineEdit(parent)
{
}

void PathLineEditor::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);

    // Then select the text by a single shot timer, so that everything will
    // be processed before (calling selectAll() directly won't work)
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
}

void PathLineEditor::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);
}