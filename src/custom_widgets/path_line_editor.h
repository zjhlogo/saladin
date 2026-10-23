// \file path_line_editor.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

class PathLineEditor : public QLineEdit
{
    Q_OBJECT

public:
    explicit PathLineEditor(QWidget* parent = nullptr);
    ~PathLineEditor() override = default;

protected:
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
};
