// \file stream_device.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include <QIODevice>

class StreamDevicePrivate;
class ShellPidl;

class StreamDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit StreamDevice(const ShellPidl& pidl);
    ~StreamDevice() override;

public:
    QString name() const;

public: // overrides
    bool open(OpenMode mode) override;

    qint64 size() const override;
    bool seek(qint64 pos) override;

    QDateTime lastModified() const;

protected: // overrides
    qint64 readData(char* data, qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 maxSize) override;

private:
    StreamDevicePrivate* d;
};
