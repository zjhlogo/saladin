// \file text_encoding.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "text_encoding.h"

#include <windows.h>

namespace
{

    struct ConverterName
    {
        const char* m_name;
        QStringConverter::Encoding m_encoding;
    };

    struct CodePageName
    {
        const char* m_name;
        unsigned int m_codePage;
    };

    // encodings handled directly by QStringDecoder
    const ConverterName converterNames[] = {
        {"UTF-8", QStringConverter::Utf8},
        {"UTF-16", QStringConverter::Utf16},
        {"UTF-16LE", QStringConverter::Utf16LE},
        {"UTF-16BE", QStringConverter::Utf16BE},
        {"UTF-32", QStringConverter::Utf32},
        {"UTF-32LE", QStringConverter::Utf32LE},
        {"UTF-32BE", QStringConverter::Utf32BE},
        {"ISO-8859-1", QStringConverter::Latin1},
        {"System", QStringConverter::System},
    };

    // legacy encodings mapped to their Windows code page
    const CodePageName codePageNames[] = {
        {"IBM850", 850},        {"IBM866", 866},        {"Windows-1250", 1250}, {"Windows-1251", 1251}, {"Windows-1252", 1252}, {"Windows-1253", 1253},
        {"Windows-1254", 1254}, {"Windows-1255", 1255}, {"Windows-1256", 1256}, {"Windows-1257", 1257}, {"Windows-1258", 1258}, {"ISO-8859-2", 28592},
        {"ISO-8859-3", 28593},  {"ISO-8859-4", 28594},  {"ISO-8859-5", 28595},  {"ISO-8859-6", 28596},  {"ISO-8859-7", 28597},  {"ISO-8859-8", 28598},
        {"ISO-8859-9", 28599},  {"ISO-8859-13", 28603}, {"ISO-8859-14", 28604}, {"ISO-8859-15", 28605}, {"ISO-8859-16", 28606}, {"KOI8-R", 20866},
        {"KOI8-U", 21866},      {"GB18030", 54936},     {"Big5", 950},          {"Big5-HKSCS", 950},    {"Shift_JIS", 932},     {"EUC-JP", 51932},
        {"ISO-2022-JP", 50220}, {"EUC-KR", 51949},      {"TIS-620", 874},
    };

    bool matchesName(const QString& name, const char* candidate)
    {
        return name.compare(QLatin1String(candidate), Qt::CaseInsensitive) == 0;
    }

    bool matchesBom(const QByteArray& header, const char* bom, int length)
    {
        if (header.length() < length) return false;

        return memcmp(header.constData(), bom, length) == 0;
    }

    // Converts a single buffer using the given code page.
    // 'error' receives the GetLastError() code of a failed conversion.
    QString convertCodePage(unsigned int codePage, const char* data, qsizetype length, bool strict, int& error)
    {
        const DWORD flags = strict ? MB_ERR_INVALID_CHARS : 0;

        const int required = MultiByteToWideChar(codePage, flags, data, static_cast<int>(length), nullptr, 0);

        if (required <= 0)
        {
            error = static_cast<int>(GetLastError());
            return QString();
        }

        QString result(required, Qt::Uninitialized);

        if (MultiByteToWideChar(codePage, flags, data, static_cast<int>(length), reinterpret_cast<LPWSTR>(result.data()), required) <= 0)
        {
            error = static_cast<int>(GetLastError());
            return QString();
        }

        error = 0;

        return result;
    }

} // namespace

TextEncoding::TextEncoding()
    : m_decoder(std::in_place, QStringConverter::System)
{
}

TextEncoding::TextEncoding(const QByteArray& formatName)
{
    const QString name = QString::fromLatin1(formatName).trimmed();

    for (int i = 0; i < static_cast<int>(sizeof(converterNames) / sizeof(converterNames[0])); i++)
    {
        if (matchesName(name, converterNames[i].m_name))
        {
            m_decoder.emplace(converterNames[i].m_encoding);
            return;
        }
    }

    for (int i = 0; i < static_cast<int>(sizeof(codePageNames) / sizeof(codePageNames[0])); i++)
    {
        if (matchesName(name, codePageNames[i].m_name))
        {
            m_codePage = codePageNames[i].m_codePage;
            return;
        }
    }

    m_decoder.emplace(QStringConverter::System);
}

QString TextEncoding::decode(QByteArrayView data)
{
    if (m_decoder) return (*m_decoder)(data);

    QByteArray input;

    const char* bytes = data.constData();
    qsizetype size = data.size();

    if (!m_pending.isEmpty())
    {
        input = m_pending;
        input.append(data.constData(), data.size());
        m_pending.clear();

        bytes = input.constData();
        size = input.size();
    }

    if (size <= 0) return QString();

    if (!m_lenient)
    {
        // an incomplete multi-byte sequence at the end of the buffer is not
        // an error - hold back up to 4 trailing bytes until the next block
        for (qsizetype trim = 0; trim <= 4 && trim < size; trim++)
        {
            int error = 0;
            QString result = convertCodePage(m_codePage, bytes, size - trim, true, error);

            if (error == 0)
            {
                if (trim > 0) m_pending = QByteArray(bytes + size - trim, trim);
                return result;
            }

            if (error != ERROR_NO_UNICODE_TRANSLATION)
            {
                // the code page does not support strict conversion
                m_lenient = true;
                break;
            }
        }
    }

    int error = 0;

    return convertCodePage(m_codePage, bytes, size, false, error);
}

QByteArray TextEncoding::systemName()
{
    return QByteArray("System");
}

bool TextEncoding::detectBom(const QByteArray& header, QByteArray& formatName)
{
    if (matchesBom(header, "\xEF\xBB\xBF", 3))
    {
        formatName = "UTF-8";
        return true;
    }

    if (matchesBom(header, "\xFF\xFE\x00\x00", 4))
    {
        formatName = "UTF-32LE";
        return true;
    }

    if (matchesBom(header, "\x00\x00\xFE\xFF", 4))
    {
        formatName = "UTF-32BE";
        return true;
    }

    if (matchesBom(header, "\xFF\xFE", 2))
    {
        formatName = "UTF-16LE";
        return true;
    }

    if (matchesBom(header, "\xFE\xFF", 2))
    {
        formatName = "UTF-16BE";
        return true;
    }

    return false;
}

bool TextEncoding::isValidUtf8(const QByteArray& data)
{
    QStringDecoder decoder(QStringConverter::Utf8);

    const QString text = decoder.decode(QByteArrayView(data));
    (void)text;

    return !decoder.hasError();
}
