/**************************************************************************
 * This file is part of the Saladin program
 * Copyright (C) 2011-2017 Michał Męciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "textloader.h"
#include "shell/streamdevice.h"
#include "utils/text_encoding.h"

TextLoader::TextLoader(const ShellPidl& pidl, const QByteArray& format)
    : m_pidl(pidl)
    , m_format(format)
    , m_aborted(false)
    , m_signal(false)
    , m_atEnd(false)
    , m_estimatedLength(0)
{
}

TextLoader::~TextLoader()
{
}

void TextLoader::run()
{
    StreamDevice file(m_pidl);

    if (file.open(QIODevice::ReadOnly))
    {
        TextEncoding decoder(m_format);

        char buffer[16 * 1024];

        qint64 length = 0;

        bool cr = false;

        while (!m_aborted && !m_atEnd)
        {
            qint64 read = file.read(buffer, sizeof(buffer));
            if (read < 0) read = 0;

            QString block = decoder.decode(QByteArrayView(buffer, read));

            if (cr && !block.isEmpty() && block.at(0) == QLatin1Char('\n')) block.remove(0, 1);

            if (!block.isEmpty()) cr = (block.at(block.length() - 1) == QLatin1Char('\r'));

            length += block.size();

            m_mutex.lock();

            bool signal = m_signal;
            m_signal = false;

            if (!block.isEmpty()) m_queue.enqueue(block);

            if (file.pos() > 0)
                m_estimatedLength = file.size() * length / file.pos();
            else
                m_estimatedLength = 0;

            if (read < (qint64)sizeof(buffer)) m_atEnd = true;

            m_mutex.unlock();

            if (signal && !m_aborted) emit nextBlockAvailable();
        }
    }
}

QString TextLoader::nextBlock()
{
    QString block;

    m_mutex.lock();

    if (!m_queue.isEmpty())
        block = m_queue.dequeue();
    else
        m_signal = true;

    m_mutex.unlock();

    return block;
}

void TextLoader::abort()
{
    m_aborted = true;

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

    if (isFinished()) deleteLater();
}
