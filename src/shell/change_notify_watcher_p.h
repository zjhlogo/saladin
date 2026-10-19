// \file change_notify_watcher_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <shlobj.h>

#include <QObject>

class ChangeNotifyWatcher : public QObject
{
    Q_OBJECT
public:
    ChangeNotifyWatcher(LPITEMIDLIST pidl, int eventTypes, QObject* parent);
    ~ChangeNotifyWatcher() override;

public:
    bool isValid() const;

signals:
    void changeNotify(int eventType, void* arg1, void* arg2);

private:
    friend LRESULT CALLBACK ChangeNotifyWatcherProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

private:
    HWND m_window;

    ULONG m_registerId;
};
