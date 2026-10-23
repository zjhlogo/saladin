// \file search_dialog.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "xmlui/client.h"

#include <QDialog>

class ShellFolder;
class FolderItemView;
class SearchItemModel;
class SearchProxyModel;
class ElidedLabel;
class QGroupBox;
class QLabel;

class SearchDialog : public QDialog, public XmlUi::Client
{
    Q_OBJECT
public:
    SearchDialog(ShellFolder* folder, QWidget* parent);
    ~SearchDialog() override;

public: // overrides
    void accept() override;
    void reject() override;
    void changeEvent(QEvent* e) override;

private:
    void retranslateUi();

private slots:
    void textToggled(bool on);

    void folderEntered(const QString& path);
    void updateResults();

    void searchCompleted();

    void contextMenuRequested(const QPoint& pos);

    void itemDoubleClicked(const QModelIndex& index);

    void viewCurrent();
    void viewAll();
    void editCurrent();
    void gotoFile();

    void copyNames();

private:
    ShellFolder* m_folder{};

    QLabel* m_promptLabel{};
    QLabel* m_locationLabel{};
    QLabel* m_patternLabel{};
    QGroupBox* m_resultsBox{};

    QLineEdit* m_locationEdit{};
    QComboBox* m_patternComboBox{};

    QCheckBox* m_textCheckBox{};
    QComboBox* m_textComboBox{};
    QCheckBox* m_caseCheckBox{};

    QDialogButtonBox* m_buttonBox{};

    FolderItemView* m_view{};

    SearchItemModel* m_model{};
    SearchProxyModel* m_proxyModel{};

    ElidedLabel* m_searchStatus{};
    ElidedLabel* m_itemsStatus{};

    bool m_searchStarted{};
};
