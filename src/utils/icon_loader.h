// \file icon_loader.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include <QIcon>
#include <QPixmap>

class IconLoader
{
public:
    static QPixmap pixmap(const QString& name, int size = 16);

    static QPixmap overlaidPixmap(const QString& name, const QString& overlay, int size = 16);
    static QPixmap overlaidPixmap(const QString& name, const QString& overlay1, const QString& overlay2, int size = 16);

    static QIcon icon(const QString& name);

    static QIcon overlaidIcon(const QString& name, const QString& overlay);
};
