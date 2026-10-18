// \file data_serializer.h
// \author zhangjianhua
// \date 2026/10/18

#pragma once

#include <QFile>

class DataSerializer
{
public:
    DataSerializer(const QString& path);
    ~DataSerializer();

public:
    bool openForReading();
    bool openForWriting();

    QDataStream& stream() { return m_stream; }

    static int dataVersion() { return m_dataVersion; }

private:
    QFile m_file;
    QDataStream m_stream;

    static int m_dataVersion;
};
