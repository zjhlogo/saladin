// \file update_client.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "update_client.h"

UpdateClient::UpdateClient(const QString& application, const QString& version, QNetworkAccessManager* manager)
    : QObject(manager)
    , m_manager(manager)
    , m_currentReply(nullptr)
    , m_timer(nullptr)
    , m_application(application)
    , m_version(version)
    , m_state(ErrorState)
{
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

UpdateClient::~UpdateClient()
{
}

void UpdateClient::setAutoUpdate(bool enabled)
{
    if (autoUpdate() == enabled) return;

    if (enabled)
    {
        m_timer = new QTimer(this);
        m_timer->start(4 * 3600 * 1000);

        connect(m_timer, SIGNAL(timeout()), this, SLOT(checkUpdate()));

        checkUpdate();
    }
    else
    {
        delete m_timer;
        m_timer = nullptr;
    }
}

void UpdateClient::checkUpdate()
{
    if (m_state == CheckingState) return;

    m_state = CheckingState;

    m_updateVersion.clear();
    m_notesUrl.clear();
    m_downloadUrl.clear();

    QUrlQuery query;
    query.addQueryItem("app", m_application);
    query.addQueryItem("ver", m_version);

    QUrl url("http://update.mimec.org/service.php");
    url.setQuery(query);

    m_currentReply = m_manager->get(QNetworkRequest(url));

    emit stateChanged();
}

void UpdateClient::finished(QNetworkReply* reply)
{
    if (reply != m_currentReply) return;

    m_currentReply = nullptr;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();

    if (reply->error() == QNetworkReply::NoError && statusCode == 200 && contentType.startsWith("text/xml"))
    {
        QDomDocument document;

        if (document.setContent(reply, false))
        {
            QDomElement version = document.documentElement().firstChildElement("version");

            if (!version.isNull())
            {
                m_state = UpdateAvailableState;

                m_updateVersion = version.attribute("id");
                m_notesUrl = QUrl(version.firstChildElement("notesUrl").text());
                m_downloadUrl = QUrl(version.firstChildElement("downloadUrl").text());
            }
            else
            {
                m_state = CurrentVersionState;
            }
        }
        else
        {
            m_state = ErrorState;
        }
    }
    else
    {
        m_state = ErrorState;
    }

    reply->deleteLater();

    emit stateChanged();
}
