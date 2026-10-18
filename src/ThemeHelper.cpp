// \file ThemeHelper.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "ThemeHelper.h"

#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

ThemeHelper::AppTheme ThemeHelper::m_appTheme{AppTheme::Unknown};

/**
 * @brief isSystemDarkTheme
 * @return true when the operating system uses the dark color scheme
 */
bool ThemeHelper::isSystemDarkTheme()
{
#ifdef Q_OS_WIN
    // Qt >= 6.5 reports the system color scheme and tracks changes at runtime.
    Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();

    if (scheme != Qt::ColorScheme::Unknown) return scheme == Qt::ColorScheme::Dark;

    // Fallback for platform themes that don't report a color scheme.
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if (settings.value("AppsUseLightTheme") == 0)
    {
        return true;
    }
#endif
    return false;
}

ThemeHelper::AppTheme ThemeHelper::calculateAppTheme(const QString& settingTheme)
{
    auto finalTheme = AppTheme::Dark;

    if (settingTheme == QLatin1String("system"))
    {
        if (!isSystemDarkTheme())
        {
            finalTheme = AppTheme::Light;
        }
    }
    else if (settingTheme == QLatin1String("light"))
    {
        finalTheme = AppTheme::Light;
    }

    return finalTheme;
}

QString ThemeHelper::iconsPath()
{
    // The dark icon set is drawn for dark backgrounds and vice versa.
    return m_appTheme == AppTheme::Dark ? ":/icons/dark" : ":/icons";
}
