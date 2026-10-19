// \file open_ftp_dialog.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QDialog>

class OpenFtpDialog : public QDialog
{
    Q_OBJECT
public:
    OpenFtpDialog(QWidget* parent);
    ~OpenFtpDialog() override;

public:
    void setPath(const QString& path);
    void setUser(const QString& user);
    void setPassword(const QString& password);

    QString path();

public: // overrides
    void accept() override;

private:
    QLineEdit* m_hostEdit{};
    QSpinBox* m_portSpinBox{};
    QLineEdit* m_pathEdit{};
    QCheckBox* m_anonymousCheckBox{};
    QLineEdit* m_userEdit{};
    QLineEdit* m_passwordEdit{};
};
