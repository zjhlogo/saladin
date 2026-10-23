// \file separator_combo_box.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QComboBox>

class SeparatorComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit SeparatorComboBox(QWidget* parent);
    ~SeparatorComboBox() override;

public:
    void addSeparator() const;

    void addParentItem(const QString& text) const;
    void addChildItem(const QString& text, const QVariant& data = QVariant()) const;
};
