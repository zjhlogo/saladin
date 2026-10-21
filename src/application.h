// \file application.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "bookmark.h"
#include "utils/theme_helper.h"

#include <QApplication>

class MainWindow;
class LocalSettings;
class PluginManager;
class UpdateClient;
class AboutBox;
class AboutBoxSection;
class GuideDialog;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int& argc, char** argv);
    ~Application() override;

public:
    QString version() const;

    QString translationsPath() const { return m_translationsPath; }

    QString locateDataFile(const QString& name);
    QString locateCacheFile(const QString& name);
    QString locateTempFile(const QString& name);

    LocalSettings* applicationSettings() const { return m_settings; }

    PluginManager* pluginManager() const { return m_pluginManager; }

    const QList<Bookmark>& bookmarks() const { return m_bookmarks; }
    void setBookmarks(const QList<Bookmark>& bookmarks);
    void addBookmark(const Bookmark& bookmark);

public slots:
    void about();

    void showQuickGuide();

    void showUpdateState();

    void openWebsite();
    void openDonations();
    void openReleaseNotes();
    void openDownloads();

    void settingsChanged();

    void updateTheme(ThemeHelper::AppTheme theme);

signals:
    void themeChanged();

private slots:
    void updateColorScheme();

private:
    bool loadTranslation(const QString& name, bool tryQtDir);

    void initializeDefaultPaths();

    void initializeSettings();

    bool checkAccess(const QString& path);

    void loadBookmarks();
    void saveBookmarks();

private:
    QString m_translationsPath;

    QString m_dataPath;
    QString m_cachePath;
    QString m_tempPath;

    LocalSettings* m_settings{};
    UpdateClient* m_updateClient{};
    PluginManager* m_pluginManager{};
    QList<Bookmark> m_bookmarks;
    QPointer<AboutBox> m_aboutBox;

    QPointer<AboutBoxSection> m_notesSection;
    QPointer<AboutBoxSection> m_helpSection;
    QPointer<AboutBoxSection> m_updateSection;
    QPointer<AboutBoxSection> m_licenseSection;

    QPointer<QPushButton> m_updateButton;

    QString m_shownVersion;

    QPointer<GuideDialog> m_guideDialog;

    ThemeHelper::AppTheme m_appliedTheme{ThemeHelper::AppTheme::Unknown};
};

extern Application* g_application;
