/**
 *
 * Respect Windows Dark/Light themes for QT windows
 * Naveen K <prime@envyen.com>
 *
 * Based on https://github.com/statiolake/neovim-qt/commit/da8eaba7f0e38b6b51f3bacd02a8cc2d1f7a34d8
 * Warning: this is not a proper solution, and Windows api can change !!
 * Stylesheet based on https://forum.qt.io/post/523388
 *
 **/
#ifndef WINDARK_H
#define WINDARK_H

#include <qwidget.h>

#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleFactory>

#ifdef Q_OS_WIN
#    include <Windows.h>
#    include <dwmapi.h>
#    pragma comment(lib, "Dwmapi.lib")
#endif

namespace winDark
{
    void setDark_qApp();
    void setDark_Titlebar(HWND hwnd);
    bool isDarkTheme();

#ifdef Q_OS_WIN
    enum PreferredAppMode
    {
        Default,
        AllowDark,
        ForceDark,
        ForceLight,
        Max
    };

    enum WINDOWCOMPOSITIONATTRIB
    {
        WCA_UNDEFINED = 0,
        WCA_NCRENDERING_ENABLED = 1,
        WCA_NCRENDERING_POLICY = 2,
        WCA_TRANSITIONS_FORCEDISABLED = 3,
        WCA_ALLOW_NCPAINT = 4,
        WCA_CAPTION_BUTTON_BOUNDS = 5,
        WCA_NONCLIENT_RTL_LAYOUT = 6,
        WCA_FORCE_ICONIC_REPRESENTATION = 7,
        WCA_EXTENDED_FRAME_BOUNDS = 8,
        WCA_HAS_ICONIC_BITMAP = 9,
        WCA_THEME_ATTRIBUTES = 10,
        WCA_NCRENDERING_EXILED = 11,
        WCA_NCADORNMENTINFO = 12,
        WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
        WCA_VIDEO_OVERLAY_ACTIVE = 14,
        WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
        WCA_DISALLOW_PEEK = 16,
        WCA_CLOAK = 17,
        WCA_CLOAKED = 18,
        WCA_ACCENT_POLICY = 19,
        WCA_FREEZE_REPRESENTATION = 20,
        WCA_EVER_UNCLOAKED = 21,
        WCA_VISUAL_OWNER = 22,
        WCA_HOLOGRAPHIC = 23,
        WCA_EXCLUDED_FROM_DDA = 24,
        WCA_PASSIVEUPDATEMODE = 25,
        WCA_USEDARKMODECOLORS = 26,
        WCA_LAST = 27
    };

    struct WINDOWCOMPOSITIONATTRIBDATA
    {
        WINDOWCOMPOSITIONATTRIB Attrib;
        PVOID pvData;
        SIZE_T cbData;
    };

    using fnAllowDarkModeForWindow = BOOL(WINAPI*)(HWND hWnd, BOOL allow);
    using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode);
    using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA*);

#endif

} // namespace winDark

#endif // WINDARK_H