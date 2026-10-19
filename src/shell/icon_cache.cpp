// \file icon_cache.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "icon_cache_p.h"

#include <shlobj.h>

static QPixmap loadIcon(LPCWSTR pattern, int attributes)
{
    SHFILEINFO info;
    DWORD_PTR list = SHGetFileInfo(pattern, attributes, &info, sizeof(info), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    QPixmap icon;

    if (list != 0)
    {
        icon = QPixmap::fromImage(QImage::fromHICON(info.hIcon));
        DestroyIcon(info.hIcon);
    }

    return icon;
}

class IconCacheGlobal
{
public:
    IconCacheGlobal();
    ~IconCacheGlobal();

public:
    QPixmap m_fileIcon;
    QPixmap m_directoryIcon;

    QMap<QString, QPixmap> m_fileIcons;
};

Q_GLOBAL_STATIC(IconCacheGlobal, iconCacheGlobal)

IconCacheGlobal::IconCacheGlobal()
{
    m_fileIcon = loadIcon(L"*.*", FILE_ATTRIBUTE_NORMAL);
    m_directoryIcon = loadIcon(L"*.*", FILE_ATTRIBUTE_DIRECTORY);
}

IconCacheGlobal::~IconCacheGlobal()
{
}

QPixmap IconCache::fileIcon()
{
    return iconCacheGlobal()->m_fileIcon;
}

QPixmap IconCache::directoryIcon()
{
    return iconCacheGlobal()->m_directoryIcon;
}

QPixmap IconCache::loadFileIcon(const QString& extension)
{
    IconCacheGlobal* g = iconCacheGlobal();

    QMap<QString, QPixmap>::const_iterator it = g->m_fileIcons.find(extension);
    if (it != g->m_fileIcons.end()) return it.value();

    QString pattern = "*." + extension;

    QPixmap icon = loadIcon(reinterpret_cast<LPCWSTR>(pattern.utf16()), FILE_ATTRIBUTE_NORMAL);

    g->m_fileIcons.insert(extension, icon);

    return icon;
}
