// \file format_helper.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

class StreamDevice;

class FormatHelper
{
public:
    static bool checkImage(StreamDevice& file, bool force, QByteArray& format);

    static bool checkText(QIODevice& file, bool force, QByteArray& format);
};
