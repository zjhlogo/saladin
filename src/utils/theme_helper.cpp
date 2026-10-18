// \file theme_helper.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "theme_helper.h"

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QHash>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>

namespace
{
    constexpr auto K_STYLESHEET_PATH = ":/themes/qt-material/%1.qss";
    constexpr auto K_THEME_XML_PATH = ":/themes/qt-material/themes/%1.xml";
    constexpr auto K_OVERRIDES_PATH = ":/themes/qt-material/saladin.qss";

    ThemeHelper::Colors darkFallbackColors()
    {
        ThemeHelper::Colors colors;
        colors.primary = QColor(0x1d, 0xe9, 0xb6);
        colors.primaryLight = QColor(0x6e, 0xff, 0xe8);
        colors.primaryText = QColor(0x00, 0x00, 0x00);
        colors.secondary = QColor(0x23, 0x26, 0x29);
        colors.secondaryLight = QColor(0x4f, 0x5b, 0x62);
        colors.secondaryDark = QColor(0x31, 0x36, 0x3b);
        colors.secondaryText = QColor(0xff, 0xff, 0xff);
        return colors;
    }

    ThemeHelper::Colors lightFallbackColors()
    {
        // light_teal.xml with qt-material's invert_secondary applied: the light
        // and dark secondary colors are swapped.
        ThemeHelper::Colors colors;
        colors.primary = QColor(0x1d, 0xe9, 0xb6);
        colors.primaryLight = QColor(0x6e, 0xff, 0xe8);
        colors.primaryText = QColor(0x3c, 0x3c, 0x3c);
        colors.secondary = QColor(0xf5, 0xf5, 0xf5);
        colors.secondaryLight = QColor(0xe6, 0xe6, 0xe6);
        colors.secondaryDark = QColor(0xff, 0xff, 0xff);
        colors.secondaryText = QColor(0x55, 0x55, 0x55);
        return colors;
    }

    QColor blendedColor(const QColor& source, const QColor& destination, double alpha)
    {
        const double red = alpha * source.red() + (1.0 - alpha) * destination.red();
        const double green = alpha * source.green() + (1.0 - alpha) * destination.green();
        const double blue = alpha * source.blue() + (1.0 - alpha) * destination.blue();
        return {static_cast<int>(0.5 + red), static_cast<int>(0.5 + green), static_cast<int>(0.5 + blue)};
    }
} // namespace

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
    QSettings settings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", QSettings::NativeFormat);
    if (settings.value("AppsUseLightTheme") == 0)
    {
        return true;
    }
#endif
    return false;
}

ThemeHelper::AppTheme ThemeHelper::calculateAppTheme(const QString& settingTheme)
{
    const QString value = settingTheme.trimmed().toLower();

    if (value == QLatin1String("dark")) return AppTheme::Dark;

    // "classic" and "white" are the values used before the Theme setting was
    // reduced to system/light/dark; they both mean the light color scheme.
    if (value == QLatin1String("light") || value == QLatin1String("classic") || value == QLatin1String("white")) return AppTheme::Light;

    return isSystemDarkTheme() ? AppTheme::Dark : AppTheme::Light;
}

QString ThemeHelper::themeName(AppTheme theme)
{
    return theme == AppTheme::Light ? QStringLiteral("light_teal") : QStringLiteral("dark_teal");
}

ThemeHelper::Colors ThemeHelper::loadColors(AppTheme theme)
{
    const Colors fallback = (theme == AppTheme::Light) ? lightFallbackColors() : darkFallbackColors();

    QFile file(QString(K_THEME_XML_PATH).arg(themeName(theme)));
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("ThemeHelper: cannot read %s, using built-in colors", qPrintable(file.fileName()));
        return fallback;
    }

    QDomDocument document;
    if (!document.setContent(&file))
    {
        qWarning("ThemeHelper: cannot parse %s, using built-in colors", qPrintable(file.fileName()));
        return fallback;
    }

    QHash<QString, QColor> values;
    const QDomElement root = document.documentElement();

    for (QDomElement element = root.firstChildElement(QStringLiteral("color")); !element.isNull();
         element = element.nextSiblingElement(QStringLiteral("color")))
    {
        const QColor color(element.text().trimmed());
        if (color.isValid()) values.insert(element.attribute(QStringLiteral("name")), color);
    }

    const auto pick = [&values](const char* name, const QColor& defaultValue) { return values.value(QString::fromLatin1(name), defaultValue); };

    Colors colors;
    colors.primary = pick("primaryColor", fallback.primary);
    colors.primaryLight = pick("primaryLightColor", fallback.primaryLight);
    colors.primaryText = pick("primaryTextColor", fallback.primaryText);
    colors.secondary = pick("secondaryColor", fallback.secondary);
    colors.secondaryLight = pick("secondaryLightColor", fallback.secondaryLight);
    colors.secondaryDark = pick("secondaryDarkColor", fallback.secondaryDark);
    colors.secondaryText = pick("secondaryTextColor", fallback.secondaryText);

    // qt-material's invert_secondary, used for the light themes.
    if (theme == AppTheme::Light) std::swap(colors.secondaryLight, colors.secondaryDark);

    return colors;
}

ThemeHelper::Colors ThemeHelper::colors(AppTheme theme)
{
    // The theme XML files are resources, so parsing them once per theme is
    // enough; the icon loader asks for the colors for every icon it creates.
    static QHash<int, Colors> cache;

    const auto cached = cache.constFind(static_cast<int>(theme));
    if (cached != cache.constEnd()) return cached.value();

    const Colors value = loadColors(theme);
    cache.insert(static_cast<int>(theme), value);

    return value;
}

QPalette ThemeHelper::palette(AppTheme theme)
{
    const Colors themeColors = colors(theme);

    const bool dark = (theme != AppTheme::Light);
    const QColor disabledText = blendedColor(themeColors.secondaryText, themeColors.secondaryDark, 0.4);

    QPalette palette;
    palette.setColor(QPalette::Window, themeColors.secondaryDark);
    palette.setColor(QPalette::WindowText, themeColors.secondaryText);
    palette.setColor(QPalette::Base, themeColors.secondary);
    palette.setColor(QPalette::AlternateBase, themeColors.secondaryDark);
    palette.setColor(QPalette::ToolTipBase, themeColors.secondary);
    palette.setColor(QPalette::ToolTipText, themeColors.secondaryText);
    palette.setColor(QPalette::Text, themeColors.secondaryText);
    palette.setColor(QPalette::Button, themeColors.secondaryDark);
    palette.setColor(QPalette::ButtonText, themeColors.secondaryText);
    palette.setColor(QPalette::BrightText, themeColors.primaryLight);
    palette.setColor(QPalette::Light, themeColors.secondaryLight);
    palette.setColor(QPalette::Midlight, themeColors.secondaryLight);
    palette.setColor(QPalette::Dark, themeColors.secondaryLight);
    palette.setColor(QPalette::Mid, themeColors.secondaryLight);
    palette.setColor(QPalette::Shadow, dark ? QColor(0, 0, 0) : QColor(0xa0, 0xa0, 0xa0));
    palette.setColor(QPalette::Highlight, themeColors.primary);
    palette.setColor(QPalette::HighlightedText, themeColors.primaryText);
    palette.setColor(QPalette::Link, themeColors.primary);
    palette.setColor(QPalette::LinkVisited, themeColors.primaryLight);
    palette.setColor(QPalette::PlaceholderText, disabledText);

    constexpr QPalette::ColorRole disabledRoles[] = {QPalette::WindowText,
                                                     QPalette::Text,
                                                     QPalette::ButtonText,
                                                     QPalette::ToolTipText,
                                                     QPalette::HighlightedText};
    for (QPalette::ColorRole role : disabledRoles)
        palette.setColor(QPalette::Disabled, role, disabledText);

    palette.setColor(QPalette::Disabled, QPalette::Highlight, themeColors.secondaryLight);
    palette.setColor(QPalette::Disabled, QPalette::Base, themeColors.secondary);
    palette.setColor(QPalette::Disabled, QPalette::Button, themeColors.secondaryDark);

    // Keep selections visible in inactive panes as well.
    constexpr QPalette::ColorRole inactiveRoles[] = {QPalette::Window,
                                                     QPalette::WindowText,
                                                     QPalette::Base,
                                                     QPalette::AlternateBase,
                                                     QPalette::ToolTipBase,
                                                     QPalette::ToolTipText,
                                                     QPalette::Text,
                                                     QPalette::Button,
                                                     QPalette::ButtonText,
                                                     QPalette::BrightText,
                                                     QPalette::Light,
                                                     QPalette::Midlight,
                                                     QPalette::Dark,
                                                     QPalette::Mid,
                                                     QPalette::Shadow,
                                                     QPalette::Highlight,
                                                     QPalette::HighlightedText,
                                                     QPalette::Link,
                                                     QPalette::LinkVisited,
                                                     QPalette::PlaceholderText};
    for (QPalette::ColorRole role : inactiveRoles)
        palette.setColor(QPalette::Inactive, role, palette.color(QPalette::Active, role));

    return palette;
}

QString ThemeHelper::renderOverrides(const Colors& themeColors)
{
    QString overrides = readResource(QLatin1String(K_OVERRIDES_PATH));
    if (overrides.isEmpty()) return overrides;

    overrides.replace(QLatin1String("@PRIMARY@"), themeColors.primary.name());
    overrides.replace(QLatin1String("@PRIMARY_LIGHT@"), themeColors.primaryLight.name());
    overrides.replace(QLatin1String("@PRIMARY_TEXT@"), themeColors.primaryText.name());
    overrides.replace(QLatin1String("@SECONDARY@"), themeColors.secondary.name());
    overrides.replace(QLatin1String("@SECONDARY_LIGHT@"), themeColors.secondaryLight.name());
    overrides.replace(QLatin1String("@SECONDARY_DARK@"), themeColors.secondaryDark.name());
    overrides.replace(QLatin1String("@SECONDARY_TEXT@"), themeColors.secondaryText.name());

    return overrides;
}

QString ThemeHelper::readResource(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("ThemeHelper: cannot read %s", qPrintable(path));
        return {};
    }

    return QString::fromUtf8(file.readAll());
}

QString ThemeHelper::styleSheet(AppTheme theme)
{
    const QString base = readResource(QString(K_STYLESHEET_PATH).arg(themeName(theme)));
    if (base.isEmpty()) return {};

    const QString overrides = renderOverrides(colors(theme));
    if (overrides.isEmpty()) return base;

    return base + QLatin1Char('\n') + overrides;
}

void ThemeHelper::applyTheme(AppTheme appTheme)
{
    if (appTheme == AppTheme::Unknown) appTheme = AppTheme::Dark;

    m_appTheme = appTheme;

    auto* application = qobject_cast<QApplication*>(QCoreApplication::instance());
    if (!application) return;

    if (QStyle* style = QStyleFactory::create(QStringLiteral("Fusion"))) QApplication::setStyle(style);

    QApplication::setPalette(palette(appTheme));
    application->setStyleSheet(styleSheet(appTheme));
}

QString ThemeHelper::iconsPath()
{
    // The dark icon set is drawn for dark backgrounds and vice versa.
    return m_appTheme == AppTheme::Dark ? ":/icons/dark" : ":/icons";
}

QColor ThemeHelper::iconColor()
{
    // Material Symbols are tinted with the regular text color, so they follow
    // the Qt-Material stylesheet exactly (and stay readable on both themes).
    return colors(m_appTheme).secondaryText;
}

QColor ThemeHelper::disabledIconColor()
{
    // Same blend as the palette's disabled text, so a disabled toolbar button
    // and its icon fade together.
    const Colors themeColors = colors(m_appTheme);
    return blendedColor(themeColors.secondaryText, themeColors.secondaryDark, 0.4);
}