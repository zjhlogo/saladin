// \file goto_dialog.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QDialog>

class GoToDialog : public QDialog
{
    Q_OBJECT
public:
    GoToDialog(int lines, int current, QWidget* parent);
    ~GoToDialog() override;

public:
    int line() const;

private:
    QSpinBox* m_spinBox;
};
