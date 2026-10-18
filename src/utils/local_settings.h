// \file local_settings.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include <QVariant>

class LocalSettings : public QObject
{
    Q_OBJECT
public:
    LocalSettings(const QString& path, QObject* parent);
    ~LocalSettings() override;

public:
    bool contains(const QString& key) const;

    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    void load();
    void save();

signals:
    void settingsChanged();

private:
    QString m_path;
    QVariantMap m_data;
};
