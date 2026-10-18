// \file openftpdialog.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "openftpdialog.h"
#include "utils/iconloader.h"
#include "xmlui/gradientwidget.h"

OpenFtpDialog::OpenFtpDialog(QWidget* parent)
    : QDialog(parent)
{
    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    auto promptWidget = new XmlUi::GradientWidget(this);
    topLayout->addWidget(promptWidget);

    auto promptLayout = new QHBoxLayout(promptWidget);
    promptLayout->setSpacing(10);

    auto promptPixmap = new QLabel(promptWidget);
    promptLayout->addWidget(promptPixmap);

    auto promptLabel = new QLabel(promptWidget);
    promptLabel->setWordWrap(true);
    promptLayout->addWidget(promptLabel, 1);

    auto separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    topLayout->addWidget(separator);

    auto mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(6);
    topLayout->addLayout(mainLayout);

    auto gridLayout = new QGridLayout();
    mainLayout->addLayout(gridLayout);

    auto hostLabel = new QLabel(tr("&Host name:"), this);
    gridLayout->addWidget(hostLabel, 0, 0);

    m_hostEdit = new QLineEdit(this);
    gridLayout->addWidget(m_hostEdit, 0, 1);

    hostLabel->setBuddy(m_hostEdit);

    auto portLabel = new QLabel(tr("Po&rt:"), this);
    gridLayout->addWidget(portLabel, 1, 0);

    auto portLayout = new QHBoxLayout();
    gridLayout->addLayout(portLayout, 1, 1);

    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(21);
    m_portSpinBox->setMinimumWidth(100);
    portLayout->addWidget(m_portSpinBox);

    portLayout->addStretch(1);

    portLabel->setBuddy(m_portSpinBox);

    auto pathLabel = new QLabel(tr("Pa&th:"), this);
    gridLayout->addWidget(pathLabel, 2, 0);

    m_pathEdit = new QLineEdit("/", this);
    gridLayout->addWidget(m_pathEdit, 2, 1);

    pathLabel->setBuddy(m_pathEdit);

    m_anonymousCheckBox = new QCheckBox(tr("&Anonymous connection"), this);
    m_anonymousCheckBox->setChecked(true);
    gridLayout->addWidget(m_anonymousCheckBox, 3, 0, 1, 2);

    auto userLabel = new QLabel(tr("&User name:"), this);
    gridLayout->addWidget(userLabel, 4, 0);

    m_userEdit = new QLineEdit(this);
    m_userEdit->setEnabled(false);
    gridLayout->addWidget(m_userEdit, 4, 1);

    userLabel->setBuddy(m_userEdit);

    auto passwordLabel = new QLabel(tr("&Password:"), this);
    gridLayout->addWidget(passwordLabel, 5, 0);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setEnabled(false);
    gridLayout->addWidget(m_passwordEdit, 5, 1);

    passwordLabel->setBuddy(m_passwordEdit);

    connect(m_anonymousCheckBox, SIGNAL(toggled(bool)), m_userEdit, SLOT(setDisabled(bool)));
    connect(m_anonymousCheckBox, SIGNAL(toggled(bool)), m_passwordEdit, SLOT(setDisabled(bool)));

    mainLayout->addSpacing(5);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setWindowTitle(tr("Connect To FTP"));
    promptPixmap->setPixmap(IconLoader::pixmap("ftp", 22));
    promptLabel->setText(tr("Enter the FTP connection details:"));

    promptLabel->setMinimumWidth(350);
    promptLabel->setFixedHeight(promptLabel->heightForWidth(350));
    setMaximumHeight(sizeHint().height());
}

OpenFtpDialog::~OpenFtpDialog()
{
}

void OpenFtpDialog::setPath(const QString& path)
{
    QUrl url(path);

    m_hostEdit->setText(url.host());
    m_portSpinBox->setValue(url.port(21));
    m_pathEdit->setText(url.path());
}

void OpenFtpDialog::setUser(const QString& user)
{
    m_anonymousCheckBox->setChecked(false);
    m_userEdit->setText(user);
    m_userEdit->setFocus();
}

void OpenFtpDialog::setPassword(const QString& password)
{
    m_passwordEdit->setText(password);
    if (password.isEmpty()) m_passwordEdit->setFocus();
}

QString OpenFtpDialog::path()
{
    QUrl url;

    url.setScheme("ftp");
    url.setHost(m_hostEdit->text());

    int port = m_portSpinBox->value();
    if (port != 21) url.setPort(port);

    url.setPath(m_pathEdit->text());

    if (!m_anonymousCheckBox->isChecked())
    {
        url.setUserName(m_userEdit->text());
        url.setPassword(m_passwordEdit->text());
    }

    return url.toString();
}

void OpenFtpDialog::accept()
{
    QString host = m_hostEdit->text();
    if (host.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid value"), tr("Host name cannot be empty."));
        return;
    }

    if (!m_anonymousCheckBox->isChecked())
    {
        QString user = m_userEdit->text();
        if (user.isEmpty())
        {
            QMessageBox::warning(this, tr("Invalid value"), tr("User name cannot be empty."));
            return;
        }

        QString password = m_passwordEdit->text();
        if (password.isEmpty())
        {
            QMessageBox::warning(this, tr("Invalid value"), tr("Password cannot be empty."));
            return;
        }
    }

    QDialog::accept();
}
