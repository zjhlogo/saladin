// \file local_settings.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "data_serializer.h"
#include "local_settings.h"

LocalSettings::LocalSettings(const QString& path, QObject* parent)
    : QObject(parent)
    , m_path(path)
{
    load();
}

LocalSettings::~LocalSettings()
{
    save();
}

bool LocalSettings::contains(const QString& key) const
{
    return m_data.contains(key);
}

void LocalSettings::setValue(const QString& key, const QVariant& value)
{
    m_data.insert(key, value);
}

QVariant LocalSettings::value(const QString& key, const QVariant& defaultValue /*= QVariant()*/) const
{
    return m_data.value(key, defaultValue);
}

void LocalSettings::load()
{
    DataSerializer serializer(m_path);

    if (!serializer.openForReading()) return;

    serializer.stream() >> m_data;
}

void LocalSettings::save()
{
    DataSerializer serializer(m_path);

    if (!serializer.openForWriting()) return;

    serializer.stream() << m_data;

    emit settingsChanged();
}
