// \file ftp_dialog.h
// \author zhangjianhua
// \date 2026/10/22
#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QSpinBox;

// FTP 连接信息对话框（由宿主的 open_ftp_dialog 移植为插件自带实现）。
class FtpDialog : public QDialog
{
    Q_OBJECT
public:
    FtpDialog(const QIcon& icon, QWidget* parent);
    ~FtpDialog() override;

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
