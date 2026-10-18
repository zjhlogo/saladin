// \file guidedialog.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "guidedialog.h"
#include "application.h"
#include "utils/icon_loader.h"
#include "xmlui/builder.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/toolstrip.h"

GuideDialog::GuideDialog(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Quick Guide"));

    QAction* action;

    action = new QAction(tr("Back"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(goBack()));
    setAction("goBack", action);

    action = new QAction(tr("Forward"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(goForward()));
    setAction("goForward", action);

    action = new QAction(tr("Home"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(goHome()));
    setAction("goHome", action);

    loadXmlUiFile(":/resources/guidedialog.xml");

    auto builder = new XmlUi::Builder(this);
    builder->addClient(this);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    auto promptWidget = new XmlUi::GradientWidget(this);
    topLayout->addWidget(promptWidget);

    auto promptLayout = new QHBoxLayout(promptWidget);
    promptLayout->setSpacing(10);

    m_promptPixmap = new QLabel(promptWidget);
    promptLayout->addWidget(m_promptPixmap);

    auto promptLabel = new QLabel(promptWidget);
    promptLabel->setWordWrap(true);
    promptLabel->setText(tr("Quick guide to Saladin:"));
    promptLabel->setMinimumWidth(350);
    promptLabel->setFixedHeight(promptLabel->heightForWidth(350));
    promptLayout->addWidget(promptLabel, 1);

    auto separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    topLayout->addWidget(separator);

    auto mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(4);
    topLayout->addLayout(mainLayout);

    auto strip = new XmlUi::ToolStrip(this);
    builder->registerToolStrip("stripGuide", strip);
    mainLayout->addWidget(strip);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenExternalLinks(true);
    mainLayout->addWidget(m_browser, 1);

    connect(m_browser, SIGNAL(historyChanged()), this, SLOT(updateActions()));

    mainLayout->addSpacing(7);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    loadIcons();

    connect(g_application, SIGNAL(themeChanged()), this, SLOT(loadIcons()));

    updateActions();

    goHome();
}

GuideDialog::~GuideDialog()
{
}

void GuideDialog::goBack()
{
    m_browser->backward();
}

void GuideDialog::goForward()
{
    m_browser->forward();
}

void GuideDialog::goHome()
{
    QString language = QLocale().name();
    QString path = QString(":/guide/%1/index.html").arg(language);

    if (!QFile::exists(path))
    {
        language = language.left(language.indexOf('_'));
        path = QString(":/guide/%1/index.html").arg(language);

        if (!QFile::exists(path)) path = ":/guide/en/index.html";
    }

    m_browser->setSource(QUrl::fromLocalFile(path));
}

void GuideDialog::updateActions()
{
    action("goBack")->setEnabled(m_browser->isBackwardAvailable());
    action("goForward")->setEnabled(m_browser->isForwardAvailable());
}

void GuideDialog::loadIcons()
{
    action("goBack")->setIcon(IconLoader::icon("arrow-left"));
    action("goForward")->setIcon(IconLoader::icon("arrow-right"));
    action("goHome")->setIcon(IconLoader::icon("home"));

    m_promptPixmap->setPixmap(IconLoader::pixmap("help", 22));
}
