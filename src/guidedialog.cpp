/**************************************************************************
 * This file is part of the Saladin program
 * Copyright (C) 2011-2017 Michał Męciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "guidedialog.h"
#include "application.h"
#include "utils/iconloader.h"
#include "windark.h"
#include "xmlui/builder.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/toolstrip.h"

GuideDialog::GuideDialog(QWidget* parent)
    : QDialog(parent)
{
    if (winDark::isDarkTheme())
    {
        winDark::setDark_Titlebar(reinterpret_cast<HWND>(winId()));
    }

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

    XmlUi::Builder* builder = new XmlUi::Builder(this);
    builder->addClient(this);

    QVBoxLayout* topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget(this);
    topLayout->addWidget(promptWidget);

    QHBoxLayout* promptLayout = new QHBoxLayout(promptWidget);
    promptLayout->setSpacing(10);

    m_promptPixmap = new QLabel(promptWidget);
    promptLayout->addWidget(m_promptPixmap);

    QLabel* promptLabel = new QLabel(promptWidget);
    promptLabel->setWordWrap(true);
    promptLabel->setText(tr("Quick guide to Saladin:"));
    promptLabel->setMinimumWidth(350);
    promptLabel->setFixedHeight(promptLabel->heightForWidth(350));
    promptLayout->addWidget(promptLabel, 1);

    QFrame* separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    topLayout->addWidget(separator);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin(9);
    mainLayout->setSpacing(4);
    topLayout->addLayout(mainLayout);

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip(this);
    builder->registerToolStrip("stripGuide", strip);
    mainLayout->addWidget(strip);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenExternalLinks(true);
    mainLayout->addWidget(m_browser, 1);

    connect(m_browser, SIGNAL(historyChanged()), this, SLOT(updateActions()));

    mainLayout->addSpacing(7);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    loadIcons();

    connect(application, SIGNAL(themeChanged()), this, SLOT(loadIcons()));

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
    action("goBack")->setIcon(IconLoader::icon("find-previous"));
    action("goForward")->setIcon(IconLoader::icon("find-next"));
    action("goHome")->setIcon(IconLoader::icon("home"));

    m_promptPixmap->setPixmap(IconLoader::pixmap("help", 22));
}
