// \file iconloader.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "iconloader.h"
#include "ThemeHelper.h"

#include <QApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QSvgRenderer>

namespace
{
    constexpr int kIconSizes[] = {16, 22, 32, 48};

    // Material Symbols are monochrome SVGs (see tools/generate_material_icons.py)
    // tinted with the active theme color, so a single set covers both the light
    // and the dark Qt-Material theme.
    QString materialIconPath(const QString& name)
    {
        return QStringLiteral(":/icons/material/%1.svg").arg(name);
    }

    bool hasMaterialIcon(const QString& name)
    {
        return QFile::exists(materialIconPath(name));
    }

    QPixmap renderMaterialIcon(const QString& name, int size, const QColor& color, qreal ratio)
    {
        QSvgRenderer renderer(materialIconPath(name));
        if (!renderer.isValid()) return QPixmap();

        const int pixels = qMax(1, qRound(size * ratio));

        QImage image(pixels, pixels, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        renderer.render(&painter, QRectF(0, 0, pixels, pixels));
        // Keep the glyph's coverage, replace the color.
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(image.rect(), color);
        painter.end();

        QPixmap pixmap = QPixmap::fromImage(image);
        pixmap.setDevicePixelRatio(ratio);
        return pixmap;
    }

    QPixmap materialPixmap(const QString& name, int size, const QColor& color)
    {
        const qreal ratio = qApp ? qApp->devicePixelRatio() : 1.0;
        const QString key = QStringLiteral("material:%1:%2:%3:%4").arg(name).arg(size).arg(color.name()).arg(ratio);

        QPixmap pixmap;
        if (QPixmapCache::find(key, &pixmap)) return pixmap;

        pixmap = renderMaterialIcon(name, size, color, ratio);
        if (!pixmap.isNull()) QPixmapCache::insert(key, pixmap);

        return pixmap;
    }

    // Icons that have no Material counterpart keep using the PNG sets in
    // ":/icons" (light) and ":/icons/dark" (dark).
    QString pngIconPath(const QString& name, int size)
    {
        return QStringLiteral("%1/%2-%3.png").arg(ThemeHelper::iconsPath(), name).arg(size);
    }
} // namespace

QPixmap IconLoader::pixmap(const QString& name, int size)
{
    if (hasMaterialIcon(name)) return materialPixmap(name, size, ThemeHelper::iconColor());

    QString key = QStringLiteral("pixmap:%1:%2").arg(name).arg(size);

    QPixmap pixmap;
    if (QPixmapCache::find(key, &pixmap)) return pixmap;

    pixmap = QPixmap(pngIconPath(name, size));

    if (!pixmap.isNull()) QPixmapCache::insert(key, pixmap);

    return pixmap;
}

QPixmap IconLoader::overlayedPixmap(const QString& name, const QString& overlay, int size)
{
    QString key = QStringLiteral("overlayed:%1:%2:%3").arg(name).arg(overlay).arg(size);

    QPixmap resultPixmap;
    if (QPixmapCache::find(key, &resultPixmap)) return resultPixmap;

    QPixmap basePixmap = pixmap(name, size);
    QPixmap overlayPixmap = pixmap(overlay, size);

    const bool valid = !basePixmap.isNull();
    resultPixmap = QPixmap(valid ? basePixmap.size() : QSize(size, size));
    resultPixmap.setDevicePixelRatio(valid ? basePixmap.devicePixelRatio() : 1.0);
    resultPixmap.fill(QColor(0, 0, 0, 0));

    QPainter painter(&resultPixmap);
    painter.drawPixmap(0, 0, basePixmap);
    painter.drawPixmap(0, 0, overlayPixmap);
    painter.end();

    QPixmapCache::insert(key, resultPixmap);

    return resultPixmap;
}

QPixmap IconLoader::overlayedPixmap(const QString& name, const QString& overlay1, const QString& overlay2, int size)
{
    QString key = QStringLiteral("overlayed2:%1:%2:%3:%4").arg(name).arg(overlay1).arg(overlay2).arg(size);

    QPixmap resultPixmap;
    if (QPixmapCache::find(key, &resultPixmap)) return resultPixmap;

    QPixmap basePixmap = pixmap(name, size);
    QPixmap overlay1Pixmap = pixmap(overlay1, size);
    QPixmap overlay2Pixmap = pixmap(overlay2, size);

    const bool valid = !basePixmap.isNull();
    resultPixmap = QPixmap(valid ? basePixmap.size() : QSize(size, size));
    resultPixmap.setDevicePixelRatio(valid ? basePixmap.devicePixelRatio() : 1.0);
    resultPixmap.fill(QColor(0, 0, 0, 0));

    QPainter painter(&resultPixmap);
    painter.drawPixmap(0, 0, basePixmap);
    painter.drawPixmap(0, 0, overlay1Pixmap);
    painter.drawPixmap(0, 0, overlay2Pixmap);
    painter.end();

    QPixmapCache::insert(key, resultPixmap);

    return resultPixmap;
}

QIcon IconLoader::icon(const QString& name)
{
    QIcon icon;

    if (hasMaterialIcon(name))
    {
        const QColor normal = ThemeHelper::iconColor();
        const QColor disabled = ThemeHelper::disabledIconColor();

        // One pixmap per size is enough: on high-DPI screens QIcon picks the
        // next larger size, which is exactly what the toolbar asks for.
        for (int size : kIconSizes)
        {
            icon.addPixmap(renderMaterialIcon(name, size, normal, 1.0), QIcon::Normal);
            icon.addPixmap(renderMaterialIcon(name, size, disabled, 1.0), QIcon::Disabled);
        }

        return icon;
    }

    for (int size : kIconSizes)
    {
        QString path = pngIconPath(name, size);

        if (QFile::exists(path)) icon.addFile(path, QSize(size, size));
    }

    return icon;
}

QIcon IconLoader::overlayedIcon(const QString& name, const QString& overlay)
{
    QIcon icon;

    for (int size : kIconSizes)
    {
        QString imagePath = pngIconPath(name, size);
        QString overlayPath = pngIconPath(overlay, size);

        if (QFile::exists(imagePath) && QFile::exists(overlayPath)) icon.addPixmap(overlayedPixmap(name, overlay, size));
    }

    return icon;
}
