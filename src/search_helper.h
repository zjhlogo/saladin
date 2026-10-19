// \file search_helper.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QThread>

class SearchHelper : public QThread
{
    Q_OBJECT
public:
    SearchHelper(const QString& pattern, Qt::CaseSensitivity cs);
    ~SearchHelper() override;

public:
    void search(const QStringList& files);

    QList<int> results();

    void abort();

signals:
    void completed();

protected: // overrides
    void run() override;

private:
    QStringMatcher m_matcher;

    QMutex m_mutex;

    QWaitCondition m_condition;

    QStringList m_files;

    int m_index{-1};
    QList<int> m_results;

    bool m_aborted{};
};

