// \file image_loader.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include "shell/shell_pidl.h"

#include <QThread>

class ImageLoader : public QThread
{
    Q_OBJECT
public:
    ImageLoader(const ShellPidl& pidl);
    ~ImageLoader() override;

public:
    QByteArray format() const { return m_format; }

    QImage image() const { return m_image; }

    void abort();

signals:
    void imageAvailable();

protected: // overrides
    void run() override;

private:
    ShellPidl m_pidl;
    QByteArray m_format;

    QImage m_image;

    bool m_aborted;
};
