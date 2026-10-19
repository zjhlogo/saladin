// \file image_loader.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "image_loader.h"
#include "shell/stream_device.h"

ImageLoader::ImageLoader(const ShellPidl& pidl)
    : m_pidl(pidl)
    , m_aborted(false)
{
}

ImageLoader::~ImageLoader()
{
}

void ImageLoader::run()
{
    StreamDevice file(m_pidl);

    if (file.open(QIODevice::ReadOnly))
    {
        QImageReader reader(&file);

        reader.setAutoTransform(true);

        m_format = reader.format();

        if (reader.read(&m_image) && !m_aborted) emit imageAvailable();
    }
}

void ImageLoader::abort()
{
    m_aborted = true;

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

    if (isFinished()) deleteLater();
}
