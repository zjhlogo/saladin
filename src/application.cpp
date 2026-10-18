// \file application.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "application.h"
#include "ThemeHelper.h"
#include "aboutbox.h"
#include "guidedialog.h"
#include "mainwindow.h"
#include "shell/shellfolder.h"
#include "shell/shellpidl.h"
#include "utils/dataserializer.h"
#include "utils/iconloader.h"
#include "utils/localsettings.h"
#include "utils/updateclient.h"
#include "xmlui/windowsstyle.h"

Application* g_application = nullptr;

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    Q_INIT_RESOURCE(icons);

    initializeDefaultPaths();

    g_application = this;

    ShellPidl::registerMetaType();

    m_settings = new LocalSettings(locateDataFile("settings.dat"), this);
    initializeSettings();

    loadBookmarks();

    QString language = m_settings->value("Language").toString();
    if (language.isEmpty()) language = QLocale::system().name();
    QLocale::setDefault(QLocale(language));

    loadTranslation("qt", true);
    loadTranslation("saladin", false);

    // The theme selected in the settings is authoritative: "light" always use the light palette, "dark" always uses the dark one and "system" follows the
    // Windows color scheme. Anything else (e.g. a missing value) is treated as "system".
    auto finalTheme = ThemeHelper::calculateAppTheme(m_settings->value("Theme").toString());
    updateTheme(finalTheme);

    // Follow the system light/dark color scheme while the application is running.
    connect(styleHints(), &QStyleHints::colorSchemeChanged, this, &Application::updateColorScheme);

    mainWindow = new MainWindow();

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    auto manager = new QNetworkAccessManager(this);

    m_updateClient = new UpdateClient("saladin", version(), manager);

    connect(m_updateClient, SIGNAL(stateChanged()), this, SLOT(showUpdateState()));

    settingsChanged();

    connect(m_settings, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));

    if (m_settings->value("LastVersion").toString() != version())
    {
        m_settings->setValue("LastVersion", version());
        QTimer::singleShot(100, this, SLOT(about()));
    }
}

Application::~Application()
{
    delete mainWindow;
    mainWindow = nullptr;

    delete m_updateSection;

    // LocalSettings emits settingsChanged() from its destructor; that must not
    // re-apply the theme while the application is shutting down.
    if (m_settings) disconnect(m_settings, nullptr, this, nullptr);

    delete m_settings;
    m_settings = nullptr;
}

void Application::about()
{
    if (!m_aboutBox)
    {
        QString message;
        message += "<h3>" + tr("Saladin %1").arg(version()) + "</h3>";
        message += "<p>" + tr("Copyright &copy; 2011-2017 Michał Męciński") + " (<a href=\"https://twitter.com/MichalMecinski\">@MichalMecinski</a>)<br>";
        message += tr("Icons copyright &copy; 2017 Łukasz Grabowski") + " (<a href=\"https://twitter.com/LukaszGx\">@LukaszGx</a>)</p>";

        QString notesMessage;
        notesMessage += "<h4>" + tr("Release Notes") + "</h4>";
        notesMessage += "<p>" + tr("See what's new in this version of Saladin:");
        notesMessage += QString(" <a href=\"http://saladin.mimec.org/release/%1\">http://saladin.mimec.org/release/%1</a></p>").arg(version());

        QString helpMessage;
        helpMessage += "<h4>" + tr("Help") + "</h4>";
        helpMessage += "<p>" + tr("Open the Saladin Quick Guide for help.") + "</p>";

        QString updateMessage;
        updateMessage += "<h4>" + tr("Latest Version") + "</h4>";
        updateMessage += "<p>" + tr("Automatic checking for latest version is disabled. You can enable it in program settings.") + "</p>";

        QString licenseMessage;
        licenseMessage += "<h4>" + tr("License") + "</h4>";
        licenseMessage += "<p>"
                          + tr("This program is free software: you can redistribute it and/or modify"
                               " it under the terms of the GNU General Public License as published by"
                               " the Free Software Foundation, either version 3 of the License, or"
                               " (at your option) any later version.")
                          + "</p>";

        m_aboutBox = new AboutBox(tr("About Saladin"), message, mainWindow);

        m_notesSection = m_aboutBox->addSection(IconLoader::pixmap("web"), notesMessage);

        m_helpSection = m_aboutBox->addSection(IconLoader::pixmap("help"), helpMessage);

        QPushButton* helpButton = m_helpSection->addButton(tr("&Quick Guide"));
        connect(helpButton, SIGNAL(clicked()), this, SLOT(showQuickGuide()));

        delete m_updateSection;

        m_updateSection = m_aboutBox->addSection(IconLoader::pixmap("info"), updateMessage);

        if (m_updateClient->autoUpdate())
        {
            showUpdateState();
        }
        else
        {
            m_updateButton = m_updateSection->addButton(tr("&Check Now"));
            connect(m_updateButton, SIGNAL(clicked()), m_updateClient, SLOT(checkUpdate()));
        }

        m_licenseSection = m_aboutBox->addSection(IconLoader::pixmap("license"), licenseMessage);
    }

    m_aboutBox->show();
    m_aboutBox->activateWindow();
}

void Application::showQuickGuide()
{
    if (m_aboutBox) m_aboutBox->close();

    if (!m_guideDialog)
    {
        m_guideDialog = new GuideDialog(mainWindow);

        m_guideDialog->resize(mainWindow->width() / 2, mainWindow->height() - 100);
        m_guideDialog->move(mainWindow->pos().x() + mainWindow->width() / 2 - 50, mainWindow->pos().y() + 50);
    }

    m_guideDialog->show();
    m_guideDialog->activateWindow();
}

void Application::showUpdateState()
{
    if (!m_updateSection)
    {
        if (m_updateClient->state() != UpdateClient::UpdateAvailableState || m_updateClient->updateVersion() == m_shownVersion) return;

        m_updateSection = new AboutBoxToolSection();
    }
    else
    {
        m_updateSection->clearButtons();
    }

    QString header = "<h4>" + tr("Latest Version") + "</h4>";

    switch (m_updateClient->state())
    {
    case UpdateClient::CheckingState:
    {
        m_updateSection->setPixmap(IconLoader::pixmap("info"));
        m_updateSection->setMessage(header + "<p>" + tr("Checking for latest version...") + "</p>");
        break;
    }

    case UpdateClient::ErrorState:
    {
        m_updateSection->setPixmap(IconLoader::pixmap("warning"));
        m_updateSection->setMessage(header + "<p>" + tr("Checking for latest version failed.") + "</p>");

        m_updateButton = m_updateSection->addButton(tr("&Retry"));
        connect(m_updateButton, SIGNAL(clicked()), m_updateClient, SLOT(checkUpdate()));
        break;
    }

    case UpdateClient::CurrentVersionState:
    {
        m_updateSection->setPixmap(IconLoader::pixmap("info"));
        m_updateSection->setMessage(header + "<p>" + tr("Your version of Saladin is up to date.") + "</p>");
        break;
    }

    case UpdateClient::UpdateAvailableState:
    {
        m_updateSection->setPixmap(IconLoader::pixmap("warning"));
        m_updateSection->setMessage(header + "<p>" + tr("The latest version of Saladin is %1.").arg(m_updateClient->updateVersion()) + "</p>");

        QPushButton* notesButton = m_updateSection->addButton(tr("&Release Notes"));
        connect(notesButton, SIGNAL(clicked()), this, SLOT(openReleaseNotes()));

        QPushButton* downloadButton = m_updateSection->addButton(tr("Do&wnload"));
        connect(downloadButton, SIGNAL(clicked()), this, SLOT(openDownloads()));

        m_shownVersion = m_updateClient->updateVersion();
        break;
    }
    }
}

void Application::openWebsite()
{
    QDesktopServices::openUrl(QString("http://saladin.mimec.org/release/%1").arg(version()));
}

void Application::openDonations()
{
    QDesktopServices::openUrl(QUrl("http://saladin.mimec.org/donations"));
}

void Application::openReleaseNotes()
{
    QDesktopServices::openUrl(m_updateClient->notesUrl());
}

void Application::openDownloads()
{
    QDesktopServices::openUrl(m_updateClient->downloadUrl());
}

static void updatePalette(QWidget* widget, const QPalette& palette)
{
    widget->setPalette(palette);
    widget->style()->polish(widget);

    foreach (QObject* child, widget->children())
    {
        if (child->isWidgetType())
        {
            auto childWidget = static_cast<QWidget*>(child);
            if (childWidget->isTopLevel())
                updatePalette(childWidget, g_application->palette());
            else
                updatePalette(childWidget, widget->palette());
        }
    }
}

void Application::settingsChanged()
{
    m_updateClient->setAutoUpdate(m_settings->value("AutoUpdate").toBool());

    // Applying the stylesheet is comparatively expensive, so only do it when
    // the theme actually changed.
    auto finalTheme = ThemeHelper::calculateAppTheme(m_settings->value("Theme").toString());
    if (finalTheme != m_appliedTheme) updateTheme(finalTheme);
}

void Application::updateTheme(ThemeHelper::AppTheme theme)
{
    ThemeHelper::applyTheme(theme);
    m_appliedTheme = theme;

    foreach (QWidget* widget, topLevelWidgets())
        updatePalette(widget, palette());

    QPixmapCache::clear();

    setWindowIcon(IconLoader::icon("saladin"));

    emit themeChanged();

    if (m_notesSection) m_notesSection->setPixmap(IconLoader::pixmap("web"));
    if (m_helpSection) m_helpSection->setPixmap(IconLoader::pixmap("help"));
    if (m_updateSection) m_updateSection->setPixmap(IconLoader::pixmap("info"));
    if (m_licenseSection) m_licenseSection->setPixmap(IconLoader::pixmap("license"));
}

QString Application::version() const
{
    return QString("1.0");
}

bool Application::loadTranslation(const QString& name, bool tryQtDir)
{
    QString fullName = name + "_" + QLocale().name();

    QStringList paths;
    paths.append(m_translationsPath);
    if (tryQtDir) paths.append(QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    foreach (QString path, paths)
    {
        auto translator = new QTranslator(this);

        if (!translator->load(fullName, path))
        {
            delete translator;
            continue;
        }

        // A catalog may load successfully and still contain no messages, e.g. a
        // stale .qm file compiled from an empty .ts. Installing it would leave
        // the user interface in English without any visible error, so treat it
        // as a missing translation and keep looking.
        if (translator->isEmpty())
        {
            qWarning("Translation catalog %s in %s contains no messages", qPrintable(fullName), qPrintable(path));
            delete translator;
            continue;
        }

        installTranslator(translator);
        return true;
    }

    return false;
}

void Application::initializeDefaultPaths()
{
    QString appPath = applicationDirPath();

    m_translationsPath = QDir::cleanPath(appPath + "/../translations");

    setApplicationName("Saladin");
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_cachePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/cache";

    m_tempPath = QDir::tempPath() + "/Saladin";
}

void Application::initializeSettings()
{
    if (!m_settings->contains("Paths_1"))
    {
        QStringList paths;
        paths.append(ShellFolder::defaultFolder().path());
        m_settings->setValue("Paths_1", QVariant::fromValue(paths.join(',')));
    }

    if (!m_settings->contains("Paths_2"))
    {
        QStringList paths;
        paths.append(ShellFolder::defaultFolder().path());
        m_settings->setValue("Paths_2", QVariant::fromValue(paths.join(',')));
    }

    if (!m_settings->contains("RememberDirectories")) m_settings->setValue("RememberDirectories", true);

    if (!m_settings->contains("HomeDirectory1"))
    {
        m_settings->setValue("HomeDirectory1", QVariant::fromValue(ShellFolder::defaultFolder().path()));
    }

    if (!m_settings->contains("HomeDirectory2"))
    {
        m_settings->setValue("HomeDirectory2", QVariant::fromValue(ShellFolder::defaultFolder().path()));
    }

    if (!m_settings->contains("BinaryFont")) m_settings->setValue("BinaryFont", "Courier New");
    if (!m_settings->contains("BinaryFontSize")) m_settings->setValue("BinaryFontSize", 10);

    if (!m_settings->contains("Theme")) m_settings->setValue("Theme", "system");

    if (!m_settings->contains("HideToolStrip")) m_settings->setValue("HideToolStrip", false);

    if (!m_settings->contains("TextFont")) m_settings->setValue("TextFont", "Courier New");
    if (!m_settings->contains("TextFontSize")) m_settings->setValue("TextFontSize", 10);

    if (!m_settings->contains("ConfirmDnd")) m_settings->setValue("ConfirmDnd", true);

    if (!m_settings->contains("AutoUpdate")) m_settings->setValue("AutoUpdate", true);

    if (!m_settings->contains("InternalViewer")) m_settings->setValue("InternalViewer", true);

    if (!m_settings->contains("EditorTool"))
    {
        wchar_t buffer[MAX_PATH];
        if (SHGetSpecialFolderPath(nullptr, buffer, CSIDL_SYSTEM, FALSE)) m_settings->setValue("EditorTool", QString::fromWCharArray(buffer) + "\\notepad.exe");
    }

    if (!m_settings->contains("ConsoleTool"))
    {
        wchar_t buffer[MAX_PATH];
        if (SHGetSpecialFolderPath(nullptr, buffer, CSIDL_SYSTEM, FALSE)) m_settings->setValue("ConsoleTool", QString::fromWCharArray(buffer) + "\\cmd.exe");
    }
}

void Application::updateColorScheme()
{
    // Resolving the setting again keeps an explicit Light/Dark choice in
    // effect; only "Follow System" yields a different theme here.
    auto finalTheme = ThemeHelper::calculateAppTheme(m_settings->value("Theme").toString());
    if (finalTheme == m_appliedTheme) return;

    // Re-apply the palette, propagate it to all widgets, clear the pixmap cache
    // and notify the windows so they can reload their theme-dependent resources.
    updateTheme(finalTheme);
}

QString Application::locateDataFile(const QString& name)
{
    QString path = m_dataPath + '/' + name;

    checkAccess(path);

    return path;
}

QString Application::locateCacheFile(const QString& name)
{
    QString path = m_cachePath + '/' + name;

    checkAccess(path);

    return path;
}

QString Application::locateTempFile(const QString& name)
{
    QString path = m_tempPath + '/' + name;

    checkAccess(path);

    return path;
}

bool Application::checkAccess(const QString& path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) return fileInfo.isReadable();

    QDir dir = fileInfo.absoluteDir();
    if (dir.exists()) return dir.isReadable();

    return dir.mkpath(dir.absolutePath());
}

void Application::setBookmarks(const QList<Bookmark>& bookmarks)
{
    m_bookmarks = bookmarks;

    saveBookmarks();
}

void Application::addBookmark(const Bookmark& bookmark)
{
    m_bookmarks.append(bookmark);

    saveBookmarks();
}

void Application::loadBookmarks()
{
    DataSerializer serializer(locateDataFile("bookmarks.dat"));

    if (!serializer.openForReading()) return;

    serializer.stream() >> m_bookmarks;
}

void Application::saveBookmarks()
{
    DataSerializer serializer(locateDataFile("bookmarks.dat"));

    if (!serializer.openForWriting()) return;

    serializer.stream() << m_bookmarks;
}
