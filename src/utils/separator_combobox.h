// \file separator_combobox.h
// \author zhangjianhua
// \date 2026/10/18

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
