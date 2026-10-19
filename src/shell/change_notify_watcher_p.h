// \file change_notify_watcher_p.h
// \author zhangjianhua
// \date 2026/10/19

#pragma once

#include <shlobj.h>
#include <shlwapi.h>

#include <QObject>

// 通知事件参数 → 被监视目录的直接子项 PIDL。
// 参数并不总是 PIDL：SHCNE_UPDATEIMAGE 的 dwItem1 为 NULL、dwItem2 是图像索引，
// SHCNE_ASSOCCHANGED 的两个参数都是 NULL，所以统一经由这里做安全检查，不要直接
// 把参数交给 ILFindChild（会读到非法指针）。
inline LPITEMIDLIST changeNotifyChildPidl(LPITEMIDLIST parent, const void* arg)
{
    if (!parent || !arg) return nullptr;

    LPITEMIDLIST child = ILFindChild(parent, (LPITEMIDLIST)arg);

    return (child && !ILIsEmpty(child)) ? child : nullptr;
}

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
