// \file format_helper.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "format_helper.h"
#include "shell/streamdevice.h"
#include "utils/text_encoding.h"

bool FormatHelper::checkImage(StreamDevice& file, bool force, QByteArray& format)
{
    QImageReader reader(&file);

    if (!force)
    {
        QByteArray format;
        QString name = file.name();
        int pos = name.lastIndexOf(QLatin1Char('.'));
        if (pos > 0) format = name.mid(pos + 1).toLower().toLatin1();
        reader.setFormat(format);
        reader.setAutoDetectImageFormat(false);
    }

    if (reader.canRead())
    {
        format = reader.format();
        return true;
    }

    return false;
}

static bool checkUtf16(const QByteArray& header, QByteArray& format)
{
    // this heuristics works for files containing mostly 0000-06FF characters,
    // which include latin, greek, cyrillic, hebrew and arabic letters

    int le = 0;
    int be = 0;

    for (int i = 0; i < header.length(); i += 2)
    {
        uchar lo = static_cast<uchar>(header.at(i));
        uchar hi = static_cast<uchar>(header.at(i + 1));

        if (lo == 0 && hi == 0 || lo == 255 && hi == 255) return false;

        if (hi == 0)
            le++;
        else if (hi > 6)
            le--;

        if (lo == 0)
            be++;
        else if (lo > 6)
            be--;
    }

    if (le > 0 && be < 0)
    {
        format = "UTF-16LE";
        return true;
    }

    if (be > 0 && le < 0)
    {
        format = "UTF-16BE";
        return true;
    }

    return false;
}

static bool checkUtf8(const QByteArray& header)
{
    return TextEncoding::isValidUtf8(header);
}

static bool checkBinary(const QByteArray& header)
{
    for (int i = 0; i < header.length(); i++)
    {
        uchar ch = static_cast<uchar>(header.at(i));
        if (ch < 9 || ch > 13 && ch < 32) return true;
    }

    return false;
}

bool FormatHelper::checkText(QIODevice& file, bool force, QByteArray& format)
{
    QByteArray header = file.peek(512);
    if (header.isEmpty()) return false;

    QByteArray bomFormat;
    if (TextEncoding::detectBom(header, bomFormat))
    {
        format = bomFormat;
        return true;
    }

    if (file.size() % 2 == 0)
    {
        if (checkUtf16(header, format)) return true;
    }

    if (!force && checkBinary(header)) return false;

    if (checkUtf8(header))
    {
        format = "UTF-8";
        return true;
    }

    format = TextEncoding::systemName();
    return true;
}
