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

#include "searchdialog.h"
#include "application.h"
#include "folderitemview.h"
#include "mainwindow.h"
#include "searchitemmodel.h"
#include "shell/shellfolder.h"
#include "utils/elidedlabel.h"
#include "utils/iconloader.h"
#include "utils/localsettings.h"
#include "viewer/viewmanager.h"
#include "windark.h"
#include "xmlui/builder.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/toolstrip.h"

SearchDialog::SearchDialog(ShellFolder* folder, QWidget* parent)
    : QDialog(parent)
    , m_folder(folder)
{
    if (winDark::isDarkTheme())
    {
        winDark::setDark_Titlebar(reinterpret_cast<HWND>(winId()));
    }

    QVBoxLayout* topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    XmlUi::GradientWidget* promptWidget = new XmlUi::GradientWidget(this);
    topLayout->addWidget(promptWidget);

    QHBoxLayout* promptLayout = new QHBoxLayout(promptWidget);
    promptLayout->setSpacing(10);

    QLabel* promptPixmap = new QLabel(promptWidget);
    promptLayout->addWidget(promptPixmap);

    QLabel* promptLabel = new QLabel(promptWidget);
    promptLabel->setWordWrap(true);
    promptLayout->addWidget(promptLabel, 1);

    QFrame* separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    topLayout->addWidget(separator);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin(9);
    mainLayout->setSpacing(6);
    topLayout->addLayout(mainLayout);

    QGridLayout* gridLayout = new QGridLayout();
    mainLayout->addLayout(gridLayout);

    QLabel* locationLabel = new QLabel(tr("Directory:"), this);
    gridLayout->addWidget(locationLabel, 0, 0);

    m_locationEdit = new QLineEdit(this);
    m_locationEdit->setReadOnly(true);
    gridLayout->addWidget(m_locationEdit, 0, 1, 1, 2);

    locationLabel->setBuddy(m_locationEdit);

    QLabel* patternLabel = new QLabel(tr("&Pattern:"), this);
    gridLayout->addWidget(patternLabel, 1, 0);

    m_patternComboBox = new QComboBox(this);
    m_patternComboBox->setEditable(true);
    m_patternComboBox->setInsertPolicy(QComboBox::NoInsert);
    gridLayout->addWidget(m_patternComboBox, 1, 1, 1, 2);

    patternLabel->setBuddy(m_patternComboBox);

    connect(m_patternComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(accept()));

    if (folder->attributes().testFlag(ShellItem::FileSystem))
    {
        m_textCheckBox = new QCheckBox(tr("&Text:"), this);
        gridLayout->addWidget(m_textCheckBox, 2, 0);

        m_textComboBox = new QComboBox(this);
        m_textComboBox->setEnabled(false);
        m_textComboBox->setEditable(true);
        m_textComboBox->setInsertPolicy(QComboBox::NoInsert);
        gridLayout->addWidget(m_textComboBox, 2, 1);

        m_caseCheckBox = new QCheckBox(tr("&Match case"), this);
        m_caseCheckBox->setEnabled(false);
        gridLayout->addWidget(m_caseCheckBox, 2, 2);

        connect(m_textCheckBox, SIGNAL(toggled(bool)), this, SLOT(textToggled(bool)));

        connect(m_textComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(accept()));
    }

    gridLayout->setColumnStretch(1, 1);

    mainLayout->addSpacing(5);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mainLayout->addWidget(m_buttonBox);

    m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Search"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QGroupBox* resultsBox = new QGroupBox(tr("Search Results"), this);
    mainLayout->addWidget(resultsBox);

    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsBox);

    QAction* action;

    action = new QAction(IconLoader::icon("view"), tr("View", "action name"), this);
    setAction("popupView", action);

    action = new QAction(IconLoader::icon("view"), tr("View Current File"), this);
    action->setShortcut(QKeySequence(Qt::Key_F3));
    connect(action, SIGNAL(triggered()), this, SLOT(viewCurrent()));
    setAction("viewCurrent", action);

    action = new QAction(IconLoader::icon("view-selected"), tr("View All Files"), this);
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3));
    connect(action, SIGNAL(triggered()), this, SLOT(viewAll()));
    setAction("viewAll", action);

    action = new QAction(IconLoader::icon("edit"), tr("Edit"), this);
    action->setShortcut(QKeySequence(Qt::Key_F4));
    connect(action, SIGNAL(triggered()), this, SLOT(editCurrent()));
    setAction("editCurrent", action);

    action = new QAction(IconLoader::icon("goto"), tr("Go To File"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    connect(action, SIGNAL(triggered()), this, SLOT(gotoFile()));
    setAction("gotoFile", action);

    action = new QAction(IconLoader::icon("copy-names"), tr("Copy File Names"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
    connect(action, SIGNAL(triggered()), this, SLOT(copyNames()));
    setAction("copyNames", action);

    setPopupMenu("popupView", "menuView", "viewCurrent");

    loadXmlUiFile(":/resources/searchdialog.xml");

    XmlUi::Builder* builder = new XmlUi::Builder(this);
    builder->addClient(this);

    XmlUi::ToolStrip* strip = new XmlUi::ToolStrip(resultsBox);
    builder->registerToolStrip("stripSearch", strip);
    strip->addAuxiliaryAction(this->action("copyNames"));
    resultsLayout->addWidget(strip);

    m_view = new FolderItemView(resultsBox);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setRootIsDecorated(false);
    m_view->setAlternatingRowColors(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setUniformRowHeights(true);
    m_view->header()->setStretchLastSection(false);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    resultsLayout->addWidget(m_view);

    connect(m_view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuRequested(const QPoint&)));
    connect(m_view, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(itemDoubleClicked(const QModelIndex&)));

    m_model = new SearchItemModel(this);

    connect(m_model, SIGNAL(folderEntered(const QString&)), this, SLOT(folderEntered(const QString&)));
    connect(m_model, SIGNAL(modelReset()), this, SLOT(updateResults()));
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(updateResults()));
    connect(m_model, SIGNAL(searchCompleted()), this, SLOT(searchCompleted()));

    m_proxyModel = new SearchProxyModel(this);
    m_proxyModel->setSortLocaleAware(true);
    m_proxyModel->setDynamicSortFilter(true);

    m_view->setModel(m_proxyModel);
    m_proxyModel->setSourceModel(m_model);

    m_view->setSortingEnabled(true);
    m_view->sortByColumn(0, Qt::AscendingOrder);

    QStatusBar* status = new QStatusBar(this);
    topLayout->addWidget(status);

    m_searchStatus = new ElidedLabel(status);
    status->addWidget(m_searchStatus, 2);

    m_itemsStatus = new ElidedLabel(status);
    status->addWidget(m_itemsStatus, 1);

    setWindowTitle(tr("Search"));
    promptPixmap->setPixmap(IconLoader::pixmap("find", 22));
    promptLabel->setText(tr("Search for files:"));

    promptLabel->setMinimumWidth(350);
    promptLabel->setFixedHeight(promptLabel->heightForWidth(350));

    m_locationEdit->setText(folder->path());

    LocalSettings* settings = application->applicationSettings();

    if (settings->contains("SearchHeaderState"))
    {
        m_view->header()->restoreState(settings->value("SearchHeaderState").toByteArray());
    }
    else
    {
        m_view->setColumnWidth(0, 410);
        m_view->setColumnWidth(1, 90);
        m_view->setColumnWidth(2, 110);
        m_view->setColumnWidth(3, 70);
    }

    m_patternComboBox->addItems(settings->value("Pattern").toStringList());
    m_patternComboBox->setCurrentIndex(0);

    if (m_textComboBox)
    {
        m_textComboBox->addItems(settings->value("FindText").toStringList());
        m_textComboBox->setCurrentIndex(0);
        m_caseCheckBox->setChecked(((QTextDocument::FindFlags)settings->value("FindFlags").toInt() & QTextDocument::FindCaseSensitively) != 0);
    }

    m_patternComboBox->setFocus();

    resize(settings->value("SearchDialogSize", QSize(750, 550)).toSize());
}

SearchDialog::~SearchDialog()
{
    LocalSettings* settings = application->applicationSettings();

    settings->setValue("SearchHeaderState", m_view->header()->saveState());
    settings->setValue("SearchDialogSize", size());
}

void SearchDialog::textToggled(bool on)
{
    m_textComboBox->setEnabled(on);
    m_caseCheckBox->setEnabled(on);

    if (on)
    {
        m_textComboBox->lineEdit()->selectAll();
        m_textComboBox->setFocus();
    }
}

void SearchDialog::accept()
{
    QString pattern = m_patternComboBox->currentText();
    if (pattern.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid value"), tr("Pattern cannot be empty."));
        return;
    }

    QString text;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;

    if (m_textCheckBox && m_textCheckBox->isChecked())
    {
        text = m_textComboBox->currentText();
        if (text.isEmpty())
        {
            QMessageBox::warning(this, tr("Invalid value"), tr("Text cannot be empty."));
            return;
        }
        cs = m_caseCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    }

    LocalSettings* settings = application->applicationSettings();

    QStringList patterns;
    patterns.append(pattern);
    for (int i = 0; i < m_patternComboBox->count(); i++)
    {
        QString item = m_patternComboBox->itemText(i);
        if (item != pattern && patterns.count() < 10) patterns.append(item);
    }
    settings->setValue("Pattern", patterns);

    m_patternComboBox->clear();
    m_patternComboBox->addItems(patterns);
    m_patternComboBox->setCurrentIndex(0);

    if (!text.isEmpty())
    {
        QStringList texts;
        texts.append(text);
        for (int i = 0; i < m_textComboBox->count(); i++)
        {
            QString item = m_textComboBox->itemText(i);
            if (item != text && texts.count() < 10) texts.append(item);
        }
        settings->setValue("FindText", texts);
        settings->setValue("FindFlags", (int)(cs == Qt::CaseSensitive ? QTextDocument::FindCaseSensitively : 0));

        m_textComboBox->clear();
        m_textComboBox->addItems(texts);
        m_textComboBox->setCurrentIndex(0);
    }

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Abort"));

    m_searchStatus->setText(tr("Searching..."));

    m_model->startSearch(m_folder, pattern, text, cs);

    m_view->setFocus();
}

void SearchDialog::reject()
{
    if (!m_model->isSearching())
    {
        QDialog::reject();
        return;
    }

    m_model->abortSearch();

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));

    m_searchStatus->setText(tr("Search aborted."));
}

void SearchDialog::updateResults()
{
    m_itemsStatus->setText(tr("%1 items found").arg(m_model->rowCount()));
}

void SearchDialog::folderEntered(const QString& path)
{
    m_searchStatus->setText(tr("Searching %1...").arg(path));
}

void SearchDialog::searchCompleted()
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));

    m_searchStatus->setText(tr("Search completed."));
}

void SearchDialog::contextMenuRequested(const QPoint& pos)
{
    if (!m_view->viewport()->visibleRegion().contains(pos)) return;

    QModelIndex index = m_proxyModel->mapToSource(m_view->indexAt(pos));

    if (index.isValid())
    {
        QList<ShellItem> items;
        items.append(m_model->itemAt(index));

        m_view->setCurrentIndex(index);

        ShellSelection selection(m_model->folderAt(index), items, this);
        selection.showContextMenu(m_view->viewport()->mapToGlobal(pos), 0);
    }
}

void SearchDialog::itemDoubleClicked(const QModelIndex& index)
{
    QModelIndex mappedIndex = m_proxyModel->mapToSource(index);

    if (!mappedIndex.isValid()) return;

    ShellFolder* folder = m_model->folderAt(mappedIndex);
    ShellItem item = m_model->itemAt(mappedIndex);

    folder->executeItem(item);
}

void SearchDialog::viewCurrent()
{
    QModelIndex index = m_proxyModel->mapToSource(m_view->currentIndex());

    ShellFolder* folder = m_model->folderAt(index);
    ShellItem item = m_model->itemAt(index);

    if (!item.isValid() || !item.attributes().testFlag(ShellItem::FileSystem)) return;

    LocalSettings* settings = application->applicationSettings();

    if (settings->value("InternalViewer").toBool())
        mainWindow->viewManager()->openView(folder->itemPidl(item));
    else
        mainWindow->startTool(MainWindow::ViewerTool, folder, item);
}

void SearchDialog::viewAll()
{
    QList<ShellPidl> pidls;

    for (int i = 0; i < m_model->rowCount(); i++)
    {
        QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(i, 0));

        ShellFolder* folder = m_model->folderAt(index);
        ShellItem item = m_model->itemAt(index);

        if (item.isValid() && item.attributes().testFlag(ShellItem::Stream)) pidls.append(folder->itemPidl(item));
    }

    if (pidls.empty()) return;

    LocalSettings* settings = application->applicationSettings();

    if (settings->value("InternalViewer").toBool())
        mainWindow->viewManager()->openView(pidls);
    else
        QMessageBox::warning(this, tr("Cannot view files"), tr("Only the internal viewer supports viewing multiple files."));
}

void SearchDialog::editCurrent()
{
    QModelIndex index = m_proxyModel->mapToSource(m_view->currentIndex());

    ShellFolder* folder = m_model->folderAt(index);
    ShellItem item = m_model->itemAt(index);

    mainWindow->startTool(MainWindow::EditorTool, folder, item);
}

void SearchDialog::gotoFile()
{
    QModelIndex index = m_proxyModel->mapToSource(m_view->currentIndex());

    ShellFolder* folder = m_model->folderAt(index);
    ShellItem item = m_model->itemAt(index);

    if (!item.isValid()) return;

    mainWindow->gotoFile(folder->pidl(), item);

    QDialog::accept();
}

void SearchDialog::copyNames()
{
    QStringList names;

    for (int i = 0; i < m_model->rowCount(); i++)
    {
        QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(i, 0));
        names.append(m_model->pathAt(index));
    }

    if (names.isEmpty()) return;

    QString text;
    if (names.count() > 1)
        text = names.join("\r\n") + "\r\n";
    else
        text = names.first();

    QApplication::clipboard()->setText(text);
}
