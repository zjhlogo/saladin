// \file goto_dialog.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "goto_dialog.h"
#include "utils/icon_loader.h"
#include "xmlui/gradient_widget.h"

GoToDialog::GoToDialog(int lines, int current, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Go To Line"));

    QVBoxLayout* topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    auto promptWidget = new XmlUi::GradientWidget(this);
    topLayout->addWidget(promptWidget);

    QHBoxLayout* promptLayout = new QHBoxLayout(promptWidget);
    promptLayout->setSpacing(10);

    auto promptPixmap = new QLabel(promptWidget);
    promptLayout->addWidget(promptPixmap);

    auto promptLabel = new QLabel(promptWidget);
    promptLabel->setWordWrap(true);
    promptLayout->addWidget(promptLabel, 1);

    QFrame* separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    topLayout->addWidget(separator);

    QBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(6);
    topLayout->addLayout(mainLayout);

    QBoxLayout* lineLayout = new QHBoxLayout();
    mainLayout->addLayout(lineLayout);

    auto label = new QLabel(tr("&Line number:"), this);
    lineLayout->addWidget(label);

    m_spinBox = new QSpinBox(this);
    m_spinBox->setRange(1, lines);
    m_spinBox->setValue(current);
    m_spinBox->setFixedWidth(150);
    lineLayout->addWidget(m_spinBox);

    label->setBuddy(m_spinBox);

    lineLayout->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    promptPixmap->setPixmap(IconLoader::pixmap("goto", 22));

    promptLabel->setText(tr("Go to the specified line (1 - %1):").arg(lines));
    promptLabel->setMinimumWidth(350);
    promptLabel->setFixedHeight(promptLabel->heightForWidth(350));

    setMaximumHeight(sizeHint().height());

    m_spinBox->selectAll();
}

GoToDialog::~GoToDialog()
{
}

int GoToDialog::line() const
{
    return m_spinBox->value();
}
