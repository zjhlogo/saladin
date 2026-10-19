// \file data_serializer.cpp
// \author zhangjianhua
// \date 2026/10/18

#include "data_serializer.h"

int DataSerializer::m_dataVersion = 0;

static constexpr int MAGIC_HEADER = 0x534CC4D3;

// increment this value every time data format is changed
static constexpr int CURRENT_VERSION = 1;

// minimum supported version
static constexpr int MINIMUM_VERSION = 1;

DataSerializer::DataSerializer(const QString& path)
    : m_file(path)
{
}

DataSerializer::~DataSerializer() = default;

bool DataSerializer::openForReading()
{
    if (!m_file.open(QIODevice::ReadOnly)) return false;

    m_stream.setDevice(&m_file);
    m_stream.setVersion(QDataStream::Qt_4_4);

    qint32 header;
    m_stream >> header;

    if (header != MAGIC_HEADER) return false;

    qint32 version;
    m_stream >> version;

    if (version < MINIMUM_VERSION || version > CURRENT_VERSION) return false;

    m_dataVersion = version;

    return true;
}

bool DataSerializer::openForWriting()
{
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    m_stream.setDevice(&m_file);
    m_stream.setVersion(QDataStream::Qt_4_4);

    m_stream << MAGIC_HEADER;
    m_stream << CURRENT_VERSION;

    m_dataVersion = CURRENT_VERSION;

    return true;
}
