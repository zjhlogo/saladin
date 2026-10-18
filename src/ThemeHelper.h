// \file ThemeHelper.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include <QColor>
#include <QPalette>
#include <QString>

/**
 * Application theme, backed by the Qt-Material stylesheets bundled in
 * ":/themes/qt-material".
 *
 * The theme XML files are the single source of truth for the colors: the QSS
 * resources are generated from them (see tools/generate_qt_material_theme.py)
 * and the palette is built from them at runtime, so custom painted widgets and
 * styled widgets always use the same colors.
 */
class ThemeHelper
{
public:
    enum class AppTheme
    {
        Unknown = 0,
        Dark,
        Light,
    };

    struct Colors
    {
        QColor primary;
        QColor primaryLight;
        QColor primaryText;
        QColor secondary;
        QColor secondaryLight;
        QColor secondaryDark;
        QColor secondaryText;
    };

public:
    // Color scheme of the operating system itself.
    static bool isSystemDarkTheme();

    // Resolves the "Theme" setting ("system", "light", "dark", and the legacy
    // "classic"/"white" values) to the theme actually in effect.
    static AppTheme calculateAppTheme(const QString& settingTheme);

    // Name of the Qt-Material theme used by the given application theme.
    static QString themeName(AppTheme theme);

    static Colors colors(AppTheme theme);
    static QPalette palette(AppTheme theme);

    // Qt-Material stylesheet plus the Saladin overrides, with the color tokens
    // replaced by the colors of the given theme.
    static QString styleSheet(AppTheme theme);

    // Installs the Fusion base style, the palette and the stylesheet.
    static void applyTheme(AppTheme appTheme);

    static QString iconsPath();

    static void setAppTheme(AppTheme appTheme) { m_appTheme = appTheme; }
    static AppTheme getAppTheme() { return m_appTheme; }

private:
    static Colors loadColors(AppTheme theme);
    static QString renderOverrides(const Colors& colors);
    static QString readResource(const QString& path);

private:
    static AppTheme m_appTheme;
};