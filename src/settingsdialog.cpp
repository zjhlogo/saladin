// \file settingsdialog.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "settingsdialog.h"
#include "application.h"
#include "shell/shellfolder.h"
#include "utils/iconloader.h"
#include "utils/localsettings.h"
#include "utils/separatorcombobox.h"
#include "xmlui/gradientwidget.h"
#include "xmlui/toolstrip.h"

#include <QAbstractItemView>

// QFontComboBox rebuilds its family model when the writing system and the font
// filters are applied, but its popup view keeps the item layout of the initial,
// still unfiltered family list. That stale layout makes the drop-down as tall as
// the whole font list and lets it be scrolled past the last family, which leaves
// a large empty area below the items. Reset the view once the list is final so
// the popup is laid out again with the filtered families.
static void resetPopupLayout(QFontComboBox* combo)
{
    if (QAbstractItemView* view = combo->view()) view->reset();
}

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Saladin Settings"));

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
    promptLabel->setText(tr("Configure settings of Saladin:"));
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
    mainLayout->addWidget(strip);

    m_generalAction = new QAction(tr("General"), this);
    m_generalAction->setCheckable(true);
    m_generalAction->setChecked(true);
    connect(m_generalAction, SIGNAL(triggered()), this, SLOT(showGeneralTab()));
    strip->addToolAction(m_generalAction);

    m_toolsAction = new QAction(tr("Tools"), this);
    m_toolsAction->setCheckable(true);
    connect(m_toolsAction, SIGNAL(triggered()), this, SLOT(showToolsTab()));
    strip->addToolAction(m_toolsAction);

    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);

    auto generalTab = new QWidget(m_stackedWidget);
    m_stackedWidget->addWidget(generalTab);

    auto generalLayout = new QVBoxLayout(generalTab);
    generalLayout->setContentsMargins(0, 6, 0, 0);

    auto regionalGroup = new QGroupBox(tr("Regional Options"), generalTab);
    auto regionalLayout = new QGridLayout(regionalGroup);
    generalLayout->addWidget(regionalGroup);

    auto languageLabel = new QLabel(tr("&Language of the user interface:"), regionalGroup);
    regionalLayout->addWidget(languageLabel, 0, 0);

    m_languageComboBox = new SeparatorComboBox(regionalGroup);
    loadLanguages();
    regionalLayout->addWidget(m_languageComboBox, 0, 1);

    languageLabel->setBuddy(m_languageComboBox);

    regionalLayout->setColumnStretch(2, 1);

    auto directoriesGroup = new QGroupBox(tr("Home Directories"), generalTab);
    auto directoriesLayout = new QGridLayout(directoriesGroup);
    generalLayout->addWidget(directoriesGroup);

    auto leftPaneLabel = new QLabel(tr("Left pane:"), directoriesGroup);
    directoriesLayout->addWidget(leftPaneLabel, 0, 0);

    m_leftPaneEdit = new QLineEdit(directoriesGroup);
    m_leftPaneEdit->setReadOnly(true);
    directoriesLayout->addWidget(m_leftPaneEdit, 0, 1);

    leftPaneLabel->setBuddy(m_leftPaneEdit);

    m_leftPaneButton = browseButton(directoriesGroup, SLOT(browseLeftPane()));
    directoriesLayout->addWidget(m_leftPaneButton, 0, 2);

    auto rightPaneLabel = new QLabel(tr("Right pane:"), directoriesGroup);
    directoriesLayout->addWidget(rightPaneLabel, 1, 0);

    m_rightPaneEdit = new QLineEdit(directoriesGroup);
    m_rightPaneEdit->setReadOnly(true);
    directoriesLayout->addWidget(m_rightPaneEdit, 1, 1);

    rightPaneLabel->setBuddy(m_rightPaneEdit);

    m_rightPaneButton = browseButton(directoriesGroup, SLOT(browseRightPane()));
    directoriesLayout->addWidget(m_rightPaneButton, 1, 2);

    m_rememberCheckBox = new QCheckBox(tr("&Remember last directories on exit"), directoriesGroup);
    directoriesLayout->addWidget(m_rememberCheckBox, 2, 1);

    auto appearanceGroup = new QGroupBox(tr("Appearance"), generalTab);
    auto appearanceLayout = new QGridLayout(appearanceGroup);
    generalLayout->addWidget(appearanceGroup);

    auto themeLabel = new QLabel(tr("&User interface theme:"), appearanceGroup);
    appearanceLayout->addWidget(themeLabel, 0, 0);

    m_themeComboBox = new SeparatorComboBox(appearanceGroup);
    m_themeComboBox->addItem(tr("Follow System"), "system");
    m_themeComboBox->addItem(tr("Light"), "light");
    m_themeComboBox->addItem(tr("Dark"), "dark");
    appearanceLayout->addWidget(m_themeComboBox, 0, 1);

    themeLabel->setBuddy(m_themeComboBox);

    appearanceLayout->setColumnStretch(2, 1);

    auto fontsGroup = new QGroupBox(tr("Fonts"), generalTab);
    auto fontsLayout = new QGridLayout(fontsGroup);
    generalLayout->addWidget(fontsGroup);

    auto binaryLabel = new QLabel(tr("&Binary view:"), fontsGroup);
    fontsLayout->addWidget(binaryLabel, 0, 0);

    m_binaryFontComboBox = new QFontComboBox(fontsGroup);
    m_binaryFontComboBox->setEditable(false);
    m_binaryFontComboBox->setWritingSystem(QFontDatabase::Latin);
    m_binaryFontComboBox->setFontFilters(QFontComboBox::ScalableFonts | QFontComboBox::MonospacedFonts);
    resetPopupLayout(m_binaryFontComboBox);
    fontsLayout->addWidget(m_binaryFontComboBox, 0, 1);

    binaryLabel->setBuddy(m_binaryFontComboBox);

    m_binaryFontSpinBox = new QSpinBox(fontsGroup);
    m_binaryFontSpinBox->setRange(8, 36);
    m_binaryFontSpinBox->setSuffix(tr(" pt"));
    fontsLayout->addWidget(m_binaryFontSpinBox, 0, 2);

    auto textLabel = new QLabel(tr("&Text view:"), fontsGroup);
    fontsLayout->addWidget(textLabel, 1, 0);

    m_textFontComboBox = new QFontComboBox(fontsGroup);
    m_textFontComboBox->setEditable(false);
    m_textFontComboBox->setWritingSystem(QFontDatabase::Latin);
    m_textFontComboBox->setFontFilters(QFontComboBox::ScalableFonts);
    resetPopupLayout(m_textFontComboBox);
    fontsLayout->addWidget(m_textFontComboBox, 1, 1);

    textLabel->setBuddy(m_textFontComboBox);

    m_textFontSpinBox = new QSpinBox(fontsGroup);
    m_textFontSpinBox->setRange(8, 36);
    m_textFontSpinBox->setSuffix(tr(" pt"));
    fontsLayout->addWidget(m_textFontSpinBox, 1, 2);

    fontsLayout->setColumnStretch(3, 1);

    auto miscGroup = new QGroupBox(tr("Misc. Options"), generalTab);
    auto miscLayout = new QVBoxLayout(miscGroup);
    generalLayout->addWidget(miscGroup);

    m_confirmDndCheckBox = new QCheckBox(tr("Confirm &Drag && Drop operations"), miscGroup);
    miscLayout->addWidget(m_confirmDndCheckBox);

    auto updateGroup = new QGroupBox(tr("Automatic Update"), generalTab);
    auto updateLayout = new QGridLayout(updateGroup);
    generalLayout->addWidget(updateGroup);

    m_updateCheckBox = new QCheckBox(tr("&Enable automatic checking for latest version of Saladin"), directoriesGroup);
    updateLayout->addWidget(m_updateCheckBox, 0, 0);

    generalLayout->addStretch(1);

    auto toolsTab = new QWidget(m_stackedWidget);
    m_stackedWidget->addWidget(toolsTab);

    auto toolsLayout = new QVBoxLayout(toolsTab);
    toolsLayout->setContentsMargins(0, 6, 0, 0);

    auto viewerGroup = new QGroupBox(tr("File Viewer"), toolsTab);
    auto viewerLayout = new QGridLayout(viewerGroup);
    toolsLayout->addWidget(viewerGroup);

    m_internalViewerCheckBox = new QCheckBox(tr("Use the internal file viewer"), viewerGroup);
    viewerLayout->addWidget(m_internalViewerCheckBox, 0, 0, 1, 2);

    m_viewerEdit = new QLineEdit(viewerGroup);
    m_viewerEdit->setReadOnly(true);
    viewerLayout->addWidget(m_viewerEdit, 1, 0);

    m_viewerButton = browseButton(viewerGroup, SLOT(browseViewer()));
    viewerLayout->addWidget(m_viewerButton, 1, 1);

    connect(m_internalViewerCheckBox, SIGNAL(toggled(bool)), m_viewerEdit, SLOT(setDisabled(bool)));
    connect(m_internalViewerCheckBox, SIGNAL(toggled(bool)), m_viewerButton, SLOT(setDisabled(bool)));

    auto editorGroup = new QGroupBox(tr("Text Editor"), toolsTab);
    auto editorLayout = new QGridLayout(editorGroup);
    toolsLayout->addWidget(editorGroup);

    m_editorEdit = new QLineEdit(editorGroup);
    m_editorEdit->setReadOnly(true);
    editorLayout->addWidget(m_editorEdit, 0, 0);

    m_editorButton = browseButton(editorGroup, SLOT(browseEditor()));
    editorLayout->addWidget(m_editorButton, 0, 1);

    auto consoleGroup = new QGroupBox(tr("Console"), toolsTab);
    auto consoleLayout = new QGridLayout(consoleGroup);
    toolsLayout->addWidget(consoleGroup);

    m_consoleEdit = new QLineEdit(consoleGroup);
    m_consoleEdit->setReadOnly(true);
    consoleLayout->addWidget(m_consoleEdit, 0, 0);

    m_consoleButton = browseButton(consoleGroup, SLOT(browseConsole()));
    consoleLayout->addWidget(m_consoleButton, 0, 1);

    auto diffGroup = new QGroupBox(tr("Compare Files"), toolsTab);
    auto diffLayout = new QGridLayout(diffGroup);
    toolsLayout->addWidget(diffGroup);

    m_diffEdit = new QLineEdit(diffGroup);
    m_diffEdit->setReadOnly(true);
    diffLayout->addWidget(m_diffEdit, 0, 0);

    m_diffButton = browseButton(diffGroup, SLOT(browseDiff()));
    diffLayout->addWidget(m_diffButton, 0, 1);

    toolsLayout->addStretch(1);

    mainLayout->addSpacing(7);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    buttonBox->button(QDialogButtonBox::Apply)->setText(tr("&Apply"));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

    loadIcons();

    connect(g_application, SIGNAL(themeChanged()), this, SLOT(loadIcons()));

    LocalSettings* settings = g_application->applicationSettings();

    int index = m_languageComboBox->findData(settings->value("Language").toString());
    if (index >= 2) m_languageComboBox->setCurrentIndex(index);

    m_pidls[0] = ShellFolder(settings->value("HomeDirectory1").toString(), this).pidl();
    m_leftPaneEdit->setText(m_pidls[0].path());

    m_pidls[1] = ShellFolder(settings->value("HomeDirectory2").toString(), this).pidl();
    m_rightPaneEdit->setText(m_pidls[1].path());

    m_rememberCheckBox->setChecked(settings->value("RememberDirectories").toBool());

    index = m_themeComboBox->findData(settings->value("Theme"));
    if (index >= 0) m_themeComboBox->setCurrentIndex(index);

    QFont binaryFont(settings->value("BinaryFont").toString());
    binaryFont.setStyleHint(QFont::Courier);
    m_binaryFontComboBox->setCurrentFont(binaryFont);
    m_binaryFontSpinBox->setValue(settings->value("BinaryFontSize").toInt());

    QFont textFont(settings->value("TextFont").toString());
    textFont.setStyleHint(QFont::Courier);
    m_textFontComboBox->setCurrentFont(textFont);
    m_textFontSpinBox->setValue(settings->value("TextFontSize").toInt());

    m_confirmDndCheckBox->setChecked(settings->value("ConfirmDnd").toBool());

    m_updateCheckBox->setChecked(settings->value("AutoUpdate").toBool());

    m_internalViewerCheckBox->setChecked(settings->value("InternalViewer").toBool());
    m_viewerEdit->setText(settings->value("ViewerTool").toString());
    m_editorEdit->setText(settings->value("EditorTool").toString());
    m_consoleEdit->setText(settings->value("ConsoleTool").toString());
    m_diffEdit->setText(settings->value("DiffTool").toString());

    resize(450, 300);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::accept()
{
    if (apply()) QDialog::accept();
}

bool SettingsDialog::apply()
{
    LocalSettings* settings = g_application->applicationSettings();

    QString language = m_languageComboBox->itemData(m_languageComboBox->currentIndex()).toString();
    if (language != settings->value("Language").toString())
        QMessageBox::warning(this, tr("Warning"), tr("Language settings will be applied when the application is restarted."));
    settings->setValue("Language", language);

    settings->setValue("HomeDirectory1", QVariant::fromValue(m_pidls[0].path()));
    settings->setValue("HomeDirectory2", QVariant::fromValue(m_pidls[1].path()));
    settings->setValue("RememberDirectories", m_rememberCheckBox->isChecked());

    settings->setValue("Theme", m_themeComboBox->currentData());

    settings->setValue("BinaryFont", m_binaryFontComboBox->currentFont().family());
    settings->setValue("BinaryFontSize", m_binaryFontSpinBox->value());
    settings->setValue("TextFont", m_textFontComboBox->currentFont().family());
    settings->setValue("TextFontSize", m_textFontSpinBox->value());

    settings->setValue("ConfirmDnd", m_confirmDndCheckBox->isChecked());

    settings->setValue("AutoUpdate", m_updateCheckBox->isChecked());

    settings->setValue("InternalViewer", m_internalViewerCheckBox->isChecked());
    settings->setValue("ViewerTool", m_viewerEdit->text());
    settings->setValue("EditorTool", m_editorEdit->text());
    settings->setValue("ConsoleTool", m_consoleEdit->text());
    settings->setValue("DiffTool", m_diffEdit->text());

    settings->save();

    return true;
}

void SettingsDialog::browseLeftPane()
{
    browsePane(m_leftPaneEdit, 0);
}

void SettingsDialog::browseRightPane()
{
    browsePane(m_rightPaneEdit, 1);
}

void SettingsDialog::browsePane(QLineEdit* edit, int index)
{
    ShellPidl result = ShellFolder::browseFolder(this, tr("Select initial directory:"), m_pidls[index]);
    if (result.isValid())
    {
        m_pidls[index] = result;
        edit->setText(result.path());
    }
}

void SettingsDialog::browseViewer()
{
    browseTool(m_viewerEdit);
}

void SettingsDialog::browseEditor()
{
    browseTool(m_editorEdit);
}

void SettingsDialog::browseConsole()
{
    browseTool(m_consoleEdit);
}

void SettingsDialog::browseDiff()
{
    browseTool(m_diffEdit);
}

void SettingsDialog::browseTool(QLineEdit* edit)
{
    QString filter = tr("Applications") + " (*.exe);;" + tr("All Files") + " (*.*)";

    QString result = QFileDialog::getOpenFileName(this, tr("Select Tool"), edit->text(), filter);
    if (!result.isEmpty()) edit->setText(QDir::toNativeSeparators(result));
}

void SettingsDialog::loadLanguages()
{
    m_languageComboBox->addItem(tr("System Default"));
    m_languageComboBox->addSeparator();

    // files in the ini format are always encoded as UTF-8 in Qt 6
    QSettings settings(g_application->translationsPath() + "/locale.ini", QSettings::IniFormat);

    settings.beginGroup("languages");
    QStringList languages = settings.allKeys();

    if (languages.isEmpty()) m_languageComboBox->addItem("English / United States", "en_US");

    foreach (QString language, languages)
    {
        QString name = settings.value(language).toString();
        m_languageComboBox->addItem(name, language);
    }

    settings.endGroup();
}

QToolButton* SettingsDialog::browseButton(QWidget* parent, const char* slot)
{
    auto button = new QToolButton(parent);
    button->setAutoRaise(true);
    button->setIconSize(QSize(16, 16));
    button->setToolTip(tr("Browse"));

    connect(button, SIGNAL(clicked()), this, slot);

    return button;
}

void SettingsDialog::showGeneralTab()
{
    m_stackedWidget->setCurrentIndex(0);

    m_generalAction->setChecked(true);
    m_toolsAction->setChecked(false);
}

void SettingsDialog::showToolsTab()
{
    m_stackedWidget->setCurrentIndex(1);

    m_generalAction->setChecked(false);
    m_toolsAction->setChecked(true);
}

void SettingsDialog::loadIcons()
{
    m_promptPixmap->setPixmap(IconLoader::pixmap("configure", 22));

    m_generalAction->setIcon(IconLoader::icon("configure"));
    m_toolsAction->setIcon(IconLoader::icon("gear"));

    m_leftPaneButton->setIcon(IconLoader::icon("browse"));
    m_rightPaneButton->setIcon(IconLoader::icon("browse"));
    m_viewerButton->setIcon(IconLoader::icon("browse"));
    m_editorButton->setIcon(IconLoader::icon("browse"));
    m_consoleButton->setIcon(IconLoader::icon("browse"));
    m_diffButton->setIcon(IconLoader::icon("browse"));
}
