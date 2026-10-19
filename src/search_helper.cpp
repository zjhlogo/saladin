// \file search_helper.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "search_helper.h"
#include "utils/format_helper.h"
#include "utils/text_encoding.h"

SearchHelper::SearchHelper(const QString& pattern, Qt::CaseSensitivity cs)
    : m_matcher(pattern, cs)
{
}

SearchHelper::~SearchHelper()
{
}

void SearchHelper::search(const QStringList& files)
{
    m_mutex.lock();

    m_files = files;
    m_results.clear();
    m_index = -1;

    m_mutex.unlock();

    m_condition.wakeAll();
}

void SearchHelper::run()
{
    m_mutex.lock();

    while (!m_aborted)
    {
        if (m_files.isEmpty()) m_condition.wait(&m_mutex);

        while (!m_files.isEmpty() && !m_aborted)
        {
            QString path = m_files.takeFirst();
            m_index++;

            m_mutex.unlock();

            bool found = false;

            QFile file(path);

            if (file.open(QIODevice::ReadOnly))
            {
                QByteArray format;

                if (FormatHelper::checkText(file, false, format))
                {
                    TextEncoding decoder(format);

                    char buffer[16 * 1024];

                    QString remainder;

                    while (!m_aborted && !found)
                    {
                        qint64 read = file.read(buffer, sizeof(buffer));
                        if (read <= 0) break;

                        QString block = remainder + decoder.decode(QByteArrayView(buffer, read));

                        if (m_matcher.indexIn(block) >= 0) found = true;

                        remainder = block.right(m_matcher.pattern().length() - 1);
                    }
                }
            }

            m_mutex.lock();

            if (found && m_index >= 0) m_results.append(m_index);
        }

        if (!m_aborted) emit completed();
    }

    m_mutex.unlock();
}

QList<int> SearchHelper::results()
{
    m_mutex.lock();

    QList<int> results = m_results;
    m_results.clear();

    m_mutex.unlock();

    return results;
}

void SearchHelper::abort()
{
    m_aborted = true;

    m_condition.wakeAll();

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

    if (isFinished()) deleteLater();
}
