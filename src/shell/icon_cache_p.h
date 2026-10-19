// \file icon_cache_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

class QPixmap;
class QString;

class IconCache
{
public:
    static QPixmap fileIcon();
    static QPixmap directoryIcon();

    static QPixmap loadFileIcon(const QString& extension);
};
