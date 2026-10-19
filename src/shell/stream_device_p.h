// \file stream_device_p.h
// \author zhangjianhua
// \date 2026/10/19
#pragma once

#include "shell_pidl.h"
#include "stream_device.h"

class StreamDevicePrivate
{
public:
    StreamDevicePrivate();
    ~StreamDevicePrivate();

public:
    StreamDevice* q;

    ShellPidl m_pidl;

    IStream* m_stream;
};
