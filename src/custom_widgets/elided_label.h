// \file elidedlabel.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include <QLabel>

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget* parent);
    ~ElidedLabel() override;

protected: // overrides
    void paintEvent(QPaintEvent* e) override;

private:
    QString m_elidedText;
    QString m_lastText;
    int m_lastWidth;
};
