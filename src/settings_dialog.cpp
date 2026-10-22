// \file settings_dialog.cpp
// \author zhangjianhua
// \date 2026/10/22

#include "settings_dialog.h"
#include "application.h"
#include "custom_widgets/side_tab_bar.h"
#include "plugin/plugin_api.h"
#include "plugin/plugin_manager.h"
#include "shell/shell_folder.h"
#include "utils/icon_loader.h"
#include "utils/local_settings.h"
#include "utils/separator_combo_box.h"

#include <QAbstractItemView>
#include <QDesktopServices>
#include <QUrl>

// QFontComboBox rebuilds its family model when the writing system and the font
// filters are applied, but its popup view keeps the item layout of the initial,
// still unfiltered family list. That stale layout makes the drop-down as tall as
// the whole font list and lets it be scrolled past the last family, which leaves
// a large empty area below the items. Reset the view once the list is final so
// the popup is laid out again with the filtered families.
static void resetPopupLayout(const QFontComboBox* combo)
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

    auto mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(9);
    topLayout->addLayout(mainLayout);

    // Navigation tabs along the left edge of the dialog.
    m_tabBar = new SideTabBar(this);
    m_tabBar->addTab(tr("General"));
    m_tabBar->addTab(tr("Tools"));
    m_tabBar->addTab(tr("Plugins"));
    mainLayout->addWidget(m_tabBar, 0, Qt::AlignTop);

    auto sideSeparator = new QFrame(this);
    sideSeparator->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    mainLayout->addWidget(sideSeparator);

    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget, 1);

    connect(m_tabBar, SIGNAL(currentChanged(int)), m_stackedWidget, SLOT(setCurrentIndex(int)));

    auto generalTab = new QWidget(m_stackedWidget);
    m_stackedWidget->addWidget(generalTab);

    auto generalLayout = new QVBoxLayout(generalTab);
    generalLayout->setContentsMargins(0, 0, 0, 0);

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
    m_themeComboBox->setMinimumWidth(150);
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
    toolsLayout->setContentsMargins(0, 0, 0, 0);

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

    auto pluginsTab = new QWidget(m_stackedWidget);
    m_stackedWidget->addWidget(pluginsTab);

    auto pluginsLayout = new QHBoxLayout(pluginsTab);
    pluginsLayout->setContentsMargins(0, 0, 0, 0);

    auto pluginListWidget = new QWidget(pluginsTab);
    auto pluginListLayout = new QVBoxLayout(pluginListWidget);
    pluginsLayout->addWidget(pluginListWidget, 1);

    m_pluginList = new QListWidget(pluginListWidget);
    pluginListLayout->addWidget(m_pluginList, 1);

    auto openPluginsFolderButton = new QPushButton(tr("&Open Plugins Folder"), pluginListWidget);
    connect(openPluginsFolderButton, SIGNAL(clicked()), this, SLOT(openPluginsFolder()));
    pluginListLayout->addWidget(openPluginsFolderButton);

    auto pluginDetailsWidget = new QWidget(pluginsTab);
    auto pluginDetailsLayout = new QVBoxLayout(pluginDetailsWidget);
    pluginsLayout->addWidget(pluginDetailsWidget, 2);

    m_pluginDetails = new QLabel(pluginDetailsWidget);
    m_pluginDetails->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_pluginDetails->setWordWrap(true);
    m_pluginDetails->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pluginDetailsLayout->addWidget(m_pluginDetails, 1);

    // 插件自定义设置控件的宿主容器；对话框销毁时控件随容器一起销毁。
    auto pluginSettingsWidget = new QWidget(pluginDetailsWidget);
    m_pluginWidgetLayout = new QVBoxLayout(pluginSettingsWidget);
    m_pluginWidgetLayout->setContentsMargins(0, 0, 0, 0);
    pluginDetailsLayout->addWidget(pluginSettingsWidget);

    auto restartLabel = new QLabel(tr("Enabling or disabling plugins takes effect after Saladin is restarted."), pluginDetailsWidget);
    restartLabel->setWordWrap(true);
    pluginDetailsLayout->addWidget(restartLabel);

    connect(m_pluginList, SIGNAL(currentRowChanged(int)), this, SLOT(pluginSelected(int)));

    populatePlugins();

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal, this);

    auto bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(9, 7, 9, 9);
    bottomLayout->addWidget(buttonBox);
    topLayout->addLayout(bottomLayout);

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

    resize(600, 360);
}

SettingsDialog::~SettingsDialog() = default;

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

    PluginManager* pluginManager = g_application->pluginManager();

    if (pluginManager)
    {
        for (int i = 0; i < m_pluginList->count(); i++)
        {
            QListWidgetItem* item = m_pluginList->item(i);
            pluginManager->setEnabled(item->data(Qt::UserRole).toString(), item->checkState() == Qt::Checked);
        }
    }

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

void SettingsDialog::loadLanguages() const
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

QToolButton* SettingsDialog::browseButton(QWidget* parent, const char* slot) const
{
    auto button = new QToolButton(parent);
    button->setAutoRaise(true);
    button->setIconSize(QSize(16, 16));
    button->setToolTip(tr("Browse"));

    connect(button, SIGNAL(clicked()), this, slot);

    return button;
}

void SettingsDialog::openPluginsFolder()
{
    PluginManager* pluginManager = g_application->pluginManager();

    if (pluginManager) QDesktopServices::openUrl(QUrl::fromLocalFile(pluginManager->userPluginsPath()));
}

void SettingsDialog::populatePlugins()
{
    PluginManager* pluginManager = g_application->pluginManager();

    if (!pluginManager) return;

    m_pluginInfos = pluginManager->pluginInfos();

    if (m_pluginInfos.isEmpty())
    {
        m_pluginList->setEnabled(false);
        m_pluginDetails->setText(tr("No plugins found.") + "<br><br>" + tr("Put each plugin into its own subfolder of the plugins folder and restart Saladin."));
        return;
    }

    foreach (PluginInfo info, m_pluginInfos)
    {
        QString label = info.name;
        if (!info.version.isEmpty()) label += "  " + info.version;

        auto item = new QListWidgetItem(label, m_pluginList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(pluginManager->isDisabledInSettings(info.id) ? Qt::Unchecked : Qt::Checked);
        item->setData(Qt::UserRole, info.id);
        item->setToolTip(info.path);
    }

    m_pluginList->setCurrentRow(0);
}

void SettingsDialog::pluginSelected(int index)
{
    updatePluginDetails(index);
}

void SettingsDialog::updatePluginDetails(int index)
{
    // 换一个插件时先移除上一个插件的设置控件（容器销毁时也随之销毁）。
    while (QLayoutItem* child = m_pluginWidgetLayout->takeAt(0))
    {
        if (QWidget* widget = child->widget()) widget->deleteLater();
        delete child;
    }

    if (index < 0 || index >= m_pluginInfos.count())
    {
        m_pluginDetails->clear();
        return;
    }

    PluginInfo info = m_pluginInfos.at(index);

    QString text = "<h4>" + info.name.toHtmlEscaped();
    if (!info.version.isEmpty()) text += " " + info.version.toHtmlEscaped();
    text += "</h4>";

    if (!info.author.isEmpty()) text += "<p>" + info.author.toHtmlEscaped() + "</p>";
    if (!info.description.isEmpty()) text += "<p>" + info.description.toHtmlEscaped() + "</p>";

    text += "<p>" + tr("File:") + " " + QDir::toNativeSeparators(info.path).toHtmlEscaped() + "</p>";
    text += "<p>" + tr("State:") + " " + pluginStateText(info) + "</p>";

    m_pluginDetails->setText(text);

    PluginManager* pluginManager = g_application->pluginManager();

    Saladin::Plugin* plugin = pluginManager ? pluginManager->loadedPlugin(info.id) : nullptr;

    if (plugin)
    {
        QWidget* settings = plugin->createSettingsWidget(m_pluginWidgetLayout->parentWidget());
        if (settings) m_pluginWidgetLayout->addWidget(settings);
    }
}

QString SettingsDialog::pluginStateText(const PluginInfo& info) const
{
    switch (info.state)
    {
    case PluginLoaded:
        return tr("Loaded");

    case PluginDisabled:
        return tr("Disabled");

    case PluginIncompatible:
        return tr("Incompatible: %1").arg(info.errorString);

    case PluginError:
        return tr("Failed to load: %1").arg(info.errorString);
    }

    return QString();
}

void SettingsDialog::loadIcons() const
{
    m_tabBar->setTabIcon(0, IconLoader::icon("configure"));
    m_tabBar->setTabIcon(1, IconLoader::icon("gear"));
    m_tabBar->setTabIcon(2, IconLoader::icon("explore"));

    m_leftPaneButton->setIcon(IconLoader::icon("browse"));
    m_rightPaneButton->setIcon(IconLoader::icon("browse"));
    m_viewerButton->setIcon(IconLoader::icon("browse"));
    m_editorButton->setIcon(IconLoader::icon("browse"));
    m_consoleButton->setIcon(IconLoader::icon("browse"));
    m_diffButton->setIcon(IconLoader::icon("browse"));
}
