// \file update_client.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include <QObject>

class UpdateClient : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        ErrorState,
        CheckingState,
        CurrentVersionState,
        UpdateAvailableState
    };

public:
    UpdateClient(const QString& application, const QString& version, QNetworkAccessManager* manager);
    ~UpdateClient() override;

public:
    void setAutoUpdate(bool enabled);
    bool autoUpdate() const { return m_timer != nullptr; }

    State state() const { return m_state; }

    const QString& updateVersion() const { return m_updateVersion; }

    const QUrl& notesUrl() const { return m_notesUrl; }
    const QUrl& downloadUrl() const { return m_downloadUrl; }

public slots:
    void checkUpdate();

signals:
    void stateChanged();

private slots:
    void finished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_currentReply;

    QTimer* m_timer;

    QString m_application;
    QString m_version;

    State m_state;

    QString m_updateVersion;
    QUrl m_notesUrl;
    QUrl m_downloadUrl;
};
