// \file text_encoding.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

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

    bool m_lenient{false};
};
