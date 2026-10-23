// \file multi_rename_widget.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <QWidget>

class MultiRenameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MultiRenameWidget(QWidget* parent);
    ~MultiRenameWidget() override;

public:
    void setInputNames(const QStringList& names);

    const QStringList& outputNames() const { return m_outputNames; }

private slots:
    void transformNames();

private:
    QStringList m_inputNames;
    QStringList m_outputNames;

    QLineEdit* m_nameEdit{};
    QLineEdit* m_extensionEdit{};
    QLineEdit* m_searchEdit{};
    QLineEdit* m_replaceEdit{};

    QCheckBox* m_regExpCheckBox{};
    QCheckBox* m_caseCheckBox{};
    QComboBox* m_caseComboBox{};

    QTreeWidget* m_list{};
};
