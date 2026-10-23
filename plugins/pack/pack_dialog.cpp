// \file pack_dialog.cpp
// \author zhangjianhua
// \date 2026/10/23

#include "pack_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

PackDialog::PackDialog(const QIcon& icon, const QString& prompt, const QString& defaultName, const QString& target, QWidget* parent)
    : QDialog(parent)
{
    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    auto promptWidget = new QWidget(this);
    topLayout->addWidget(promptWidget);

    auto promptLayout = new QHBoxLayout(promptWidget);
    promptLayout->setSpacing(10);

    auto promptPixmap = new QLabel(promptWidget);
    promptPixmap->setPixmap(icon.pixmap(22, 22));
    promptLayout->addWidget(promptPixmap);

    auto promptLabel = new QLabel(prompt, promptWidget);
    promptLabel->setTextFormat(Qt::RichText);
    promptLabel->setWordWrap(true);
    promptLabel->setMinimumWidth(350);
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

    auto nameLabel = new QLabel(tr("&Name:"), this);
    gridLayout->addWidget(nameLabel, 0, 0);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(defaultName);
    gridLayout->addWidget(m_nameEdit, 0, 1);

    nameLabel->setBuddy(m_nameEdit);

    auto targetLabel = new QLabel(tr("Target:"), this);
    gridLayout->addWidget(targetLabel, 1, 0);

    auto targetEdit = new QLineEdit(this);
    targetEdit->setText(target);
    targetEdit->setReadOnly(true);
    gridLayout->addWidget(targetEdit, 1, 1);

    targetLabel->setBuddy(targetEdit);

    m_overwriteCheckBox = new QCheckBox(tr("O&verwrite without prompt"), this);
    gridLayout->addWidget(m_overwriteCheckBox, 2, 1);

    mainLayout->addSpacing(5);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // 默认选中主文件名（不含扩展名），与宿主 OperationDialog::setName 的行为一致。
    int pos = defaultName.lastIndexOf('.');
    if (pos <= 0 || defaultName.contains(QRegularExpression("[:\\\\/]"))) pos = defaultName.length();
    m_nameEdit->setSelection(0, pos);
    m_nameEdit->setFocus();

    setWindowTitle(tr("Pack To Zip"));
    setMaximumHeight(sizeHint().height());
}

QString PackDialog::name() const
{
    return m_nameEdit ? m_nameEdit->text() : QString();
}

bool PackDialog::overwrite() const
{
    return m_overwriteCheckBox && m_overwriteCheckBox->isChecked();
}

void PackDialog::accept()
{
    if (name().trimmed().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid value"), tr("Name cannot be empty."));
        return;
    }

    QDialog::accept();
}
