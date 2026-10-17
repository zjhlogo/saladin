/**
 *
 * Respect Windows Dark/Light themes for QT windows
 * Naveen K <prime@envyen.com>
 *
 * Based on https://github.com/statiolake/neovim-qt/commit/da8eaba7f0e38b6b51f3bacd02a8cc2d1f7a34d8
 * Warning: this is not a proper solution, and Windows api can change !!
 * Stylesheet based on https://forum.qt.io/post/523388
 *
 * Dark theme detection now uses QStyleHints::colorScheme (Qt >= 6.5)
 * with the registry read kept as a fallback; see Application::initializePalette()
 * for the dark/light theme application and runtime following. An explicit
 * application theme ("Classic"/"White"/"Dark") overrides the system color
 * scheme through setThemeOverride().
 *
 **/

#include <winDark.h>

#include <QApplication>
#include <QGuiApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleHints>

namespace winDark
{
    // -1: follow the system color scheme, 0: force light, 1: force dark.
    // Set by Application::initializePalette() from the "Theme" setting.
    static int s_themeOverride = -1;

    /**
     * @brief setThemeOverride
     * @param mode -1 to follow the system color scheme, 0 to force the light
     *             theme, 1 to force the dark theme
     */
    void setThemeOverride(int mode)
    {
        s_themeOverride = (mode < 0) ? -1 : (mode > 0 ? 1 : 0);
    }

    /**
     * @brief isDarkTheme
     * @return true when the dark theme is in effect, i.e. the theme selected
     *         in the settings when it is explicit, the system color scheme
     *         otherwise
     */
    bool isDarkTheme()
    {
        if (s_themeOverride >= 0) return s_themeOverride == 1;

        return isSystemDarkTheme();
    }

    /**
     * @brief isSystemDarkTheme
     * @return true when the operating system uses the dark color scheme
     */
    bool isSystemDarkTheme()
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

    /**
     * @brief darkPalette
     * @return the dark palette to be set on the application
     */
    QPalette darkPalette()
    {
        QPalette darkPalette;
        QColor darkColor = QColor(25, 25, 25);
        QColor disabledColor = QColor(127, 127, 127);
        darkPalette.setColor(QPalette::Light, darkColor);
        darkPalette.setColor(QPalette::Midlight, darkColor);
        darkPalette.setColor(QPalette::Dark, darkColor);
        darkPalette.setColor(QPalette::Mid, darkColor);
        darkPalette.setColor(QPalette::Shadow, darkColor);
        darkPalette.setColor(QPalette::LinkVisited, darkColor);
        darkPalette.setColor(QPalette::NoRole, darkColor);

        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

        return darkPalette;
    }

    /**
     * @brief setDark_Titlebar
     * @param hwnd
     * @param dark true to use dark title bar colors, false to restore light ones
     * @details Use for each window like
     *    if(winDark::isDarkTheme()) winDark::setDark_Titlebar(reinterpret_cast<HWND>(winId()));
     */
    void setDark_Titlebar(HWND hwnd, bool dark)
    {
#ifdef Q_OS_WIN
        HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");

        if (!hUxtheme || !hUser32) return;

        fnAllowDarkModeForWindow AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
        fnSetPreferredAppMode SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));
        fnSetWindowCompositionAttribute SetWindowCompositionAttribute = reinterpret_cast<fnSetWindowCompositionAttribute>(
            GetProcAddress(hUser32, "SetWindowCompositionAttribute"));

        if (!AllowDarkModeForWindow || !SetPreferredAppMode || !SetWindowCompositionAttribute) return;

        SetPreferredAppMode(AllowDark);
        BOOL useDarkColors = dark ? TRUE : FALSE;
        AllowDarkModeForWindow(hwnd, useDarkColors);
        WINDOWCOMPOSITIONATTRIBDATA data = {WCA_USEDARKMODECOLORS, &useDarkColors, sizeof(useDarkColors)};
        SetWindowCompositionAttribute(hwnd, &data);
#endif
    }

} // namespace winDark