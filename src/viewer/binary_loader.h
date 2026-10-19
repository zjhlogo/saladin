// \file binary_loader.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_pidl.h"

#include <QThread>

class BinaryLoader : public QThread
{
    Q_OBJECT
public:
    BinaryLoader(const ShellPidl& pidl, bool hexMode);
    ~BinaryLoader() override;

public:
    QString nextBlock();

    bool atEnd() const { return m_atEnd; }

    qint64 size() const { return m_size; }

    qint64 estimatedLength() const { return m_estimatedLength; }

    void abort();

signals:
    void nextBlockAvailable();

protected: // overrides
    void run() override;

private:
    ShellPidl m_pidl;
    bool m_hexMode;

    QMutex m_mutex;

    QQueue<QString> m_queue;

    bool m_aborted;
    bool m_signal;

    bool m_atEnd;
    qint64 m_size;
    qint64 m_estimatedLength;
};
