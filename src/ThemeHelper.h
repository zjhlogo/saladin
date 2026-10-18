// \file ThemeHelper.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

class ThemeHelper
{
public:
    enum class AppTheme
    {
        Unknown = 0,
        Dark,
        Light,
    };

public:
    // Color scheme of the operating system itself.
    static bool isSystemDarkTheme();
    
    static AppTheme calculateAppTheme(const QString& settingTheme);
    static QString iconsPath();

    static void setAppTheme(AppTheme appTheme) { m_appTheme = appTheme; }
    static AppTheme getAppTheme() { return m_appTheme; }

private:
    static AppTheme m_appTheme;
};
