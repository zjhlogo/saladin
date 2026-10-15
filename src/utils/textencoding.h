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

#ifndef TEXTENCODING_H
#define TEXTENCODING_H

#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QStringConverter>

#include <optional>

/**
 * Decoder of file contents replacing QTextCodec, which was removed in Qt 6.
 *
 * Unicode encodings are handled by QStringDecoder. All remaining encodings
 * are mapped to the matching Windows code page and converted using
 * MultiByteToWideChar(), which is sufficient because Saladin is a
 * Windows-only application.
 *
 * The decoder is stateful: incomplete multi-byte sequences at the end of
 * a buffer are held back until the next call to decode().
 */
class TextEncoding
{
public:
    TextEncoding();
    explicit TextEncoding(const QByteArray& formatName);

    QString decode(QByteArrayView data);

    static QByteArray systemName();
    static bool detectBom(const QByteArray& header, QByteArray& formatName);
    static bool isValidUtf8(const QByteArray& data);

private:
    std::optional<QStringDecoder> m_decoder;

    unsigned int m_codePage{};

    QByteArray m_pending;

    bool m_lenient{ false };
};

#endif
