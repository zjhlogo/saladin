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

#include "streamdevice.h"
#include "shellpidl_p.h"
#include "streamdevice_p.h"

StreamDevice::StreamDevice(const ShellPidl& pidl)
    : d(new StreamDevicePrivate())
{
    d->q = this;

    d->m_pidl = pidl;

    IBindCtx* bindContext;
    CreateBindCtx(0, &bindContext);

    BIND_OPTS bindOpts;
    bindOpts.cbStruct = sizeof(BIND_OPTS);
    bindOpts.grfFlags = 0;
    bindOpts.grfMode = STGM_READ | STGM_SHARE_DENY_NONE;
    bindOpts.dwTickCountDeadline = 0;

    bindContext->SetBindOptions(&bindOpts);

    SHBindToObject(nullptr, pidl.d->pidl(), bindContext, IID_PPV_ARGS(&d->m_stream));

    bindContext->Release();
}

StreamDevice::~StreamDevice()
{
    delete d;
}

StreamDevicePrivate::StreamDevicePrivate()
    : q(nullptr)
    , m_stream(nullptr)
{
}

StreamDevicePrivate::~StreamDevicePrivate()
{
    if (m_stream)
    {
        m_stream->Release();
        m_stream = nullptr;
    }
}

QString StreamDevice::name() const
{
    QString name;

    if (!d->m_pidl.path().isEmpty())
    {
        QFileInfo info(d->m_pidl.path());
        name = info.fileName();
    }
    else if (d->m_stream)
    {
        STATSTG stat = {};
        HRESULT hr = d->m_stream->Stat(&stat, STATFLAG_DEFAULT);

        if (SUCCEEDED(hr))
        {
            name = QString::fromWCharArray(stat.pwcsName);

            CoTaskMemFree(stat.pwcsName);
        }
    }

    return name;
}

bool StreamDevice::open(OpenMode mode)
{
    if (!d->m_stream) return false;
    if (mode & WriteOnly) return false;

    return QIODevice::open(mode);
}

qint64 StreamDevice::size() const
{
    if (d->m_stream)
    {
        STATSTG stat = {};
        HRESULT hr = d->m_stream->Stat(&stat, STATFLAG_NONAME);

        if (SUCCEEDED(hr)) return stat.cbSize.QuadPart;
    }

    return -1;
}

bool StreamDevice::seek(qint64 pos)
{
    if (d->m_stream)
    {
        QIODevice::seek(pos);

        LARGE_INTEGER lipos;
        lipos.QuadPart = pos;

        HRESULT hr = d->m_stream->Seek(lipos, STREAM_SEEK_SET, nullptr);

        if (SUCCEEDED(hr)) return true;
    }

    return false;
}

static QDateTime systemTimeToQDateTime(const SYSTEMTIME* systemTime)
{
    SYSTEMTIME localTime;
    SystemTimeToTzSpecificLocalTime(nullptr, systemTime, &localTime);

    QDate date(localTime.wYear, localTime.wMonth, localTime.wDay);
    QTime time(localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);

    return QDateTime(date, time, Qt::LocalTime);
}

static QDateTime fileTimeToQDateTime(const FILETIME* fileTime)
{
    SYSTEMTIME systemTime;
    FileTimeToSystemTime(fileTime, &systemTime);

    return systemTimeToQDateTime(&systemTime);
}

QDateTime StreamDevice::lastModified() const
{
    if (d->m_stream)
    {
        STATSTG stat = {};
        HRESULT hr = d->m_stream->Stat(&stat, STATFLAG_NONAME);

        if (SUCCEEDED(hr)) return fileTimeToQDateTime(&stat.mtime);
    }

    return QDateTime();
}

qint64 StreamDevice::readData(char* data, qint64 maxSize)
{
    if (d->m_stream)
    {
        ULONG read = 0;
        HRESULT hr = d->m_stream->Read(data, maxSize, &read);

        if (SUCCEEDED(hr)) return read;
    }

    return -1;
}

qint64 StreamDevice::writeData(const char* /*data*/, qint64 /*maxSize*/)
{
    return -1;
}
