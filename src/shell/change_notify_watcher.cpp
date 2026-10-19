// \file change_notify_watcher.cpp
// \author zhangjianhua
// \date 2026/10/19

#include "change_notify_watcher_p.h"

static constexpr wchar_t ChangeNotifyWatcher_Window[] = L"ChangeNotifyWatcher_Window";
static constexpr wchar_t ChangeNotifyWatcher_Message[] = L"ChangeNotifyWatcher_Message";

LRESULT CALLBACK ChangeNotifyWatcherProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

// 事件是否在 dwItem1 / dwItem2 中携带 PIDL。按文档：SHCNE_UPDATEIMAGE 的 dwItem1 为
// NULL 而 dwItem2 是系统图像列表的索引，SHCNE_ASSOCCHANGED 的两个参数都是 NULL，
// 只有重命名类事件才有第二个 PIDL。对非 PIDL 参数调用 ILClone 会解引用非法指针。
static bool eventHasItem1Pidl(long eventType)
{
    switch (eventType)
    {
    case SHCNE_UPDATEIMAGE:
    case SHCNE_ASSOCCHANGED:
    case SHCNE_EXTENDED_EVENT:
        return false;
    default:
        return true;
    }
}

static bool eventHasItem2Pidl(long eventType)
{
    return eventType == SHCNE_RENAMEITEM || eventType == SHCNE_RENAMEFOLDER;
}

class ChangeNotifyWatcherGlobal
{
public:
    ChangeNotifyWatcherGlobal();
    ~ChangeNotifyWatcherGlobal();

public:
    UINT m_message;

    QMap<HWND, ChangeNotifyWatcher*> m_watchers;
};

ChangeNotifyWatcherGlobal::ChangeNotifyWatcherGlobal()
{
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = ChangeNotifyWatcherProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = ChangeNotifyWatcher_Window;

    RegisterClass(&wc);

    m_message = RegisterWindowMessage(ChangeNotifyWatcher_Message);
}

ChangeNotifyWatcherGlobal::~ChangeNotifyWatcherGlobal()
{
}

Q_GLOBAL_STATIC(ChangeNotifyWatcherGlobal, changeNotifyWatcherGlobal)

ChangeNotifyWatcher::ChangeNotifyWatcher(LPITEMIDLIST pidl, int eventTypes, QObject* parent)
    : QObject(parent)
    , m_window(nullptr)
    , m_registerId(0)
{
    ChangeNotifyWatcherGlobal* g = changeNotifyWatcherGlobal();

    m_window = CreateWindow(ChangeNotifyWatcher_Window, ChangeNotifyWatcher_Window, 0, 0, 0, 0, 0, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    if (m_window)
    {
        SHChangeNotifyEntry entry;
        entry.pidl = pidl;
        entry.fRecursive = false;

        int sources = SHCNRF_InterruptLevel | SHCNRF_ShellLevel | SHCNRF_NewDelivery;

        m_registerId = SHChangeNotifyRegister(m_window, sources, eventTypes, g->m_message, 1, &entry);

        if (m_registerId) g->m_watchers.insert(m_window, this);
    }
}

ChangeNotifyWatcher::~ChangeNotifyWatcher()
{
    ChangeNotifyWatcherGlobal* g = changeNotifyWatcherGlobal();

    if (m_registerId)
    {
        SHChangeNotifyDeregister(m_registerId);

        g->m_watchers.remove(m_window);
    }

    if (m_window) DestroyWindow(m_window);
}

bool ChangeNotifyWatcher::isValid() const
{
    return m_registerId != 0;
}

LRESULT CALLBACK ChangeNotifyWatcherProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    ChangeNotifyWatcherGlobal* g = changeNotifyWatcherGlobal();

    if (message == g->m_message)
    {
        ChangeNotifyWatcher* watcher = g->m_watchers.value(window);

        if (watcher)
        {
            long eventType = 0;
            LPITEMIDLIST* args = nullptr;
            HANDLE lock = SHChangeNotification_Lock((HANDLE)wparam, (DWORD)lparam, &args, &eventType);

            if (lock)
            {
                // 事件类型可能带 SHCNE_INTERRUPT 修饰位，先归一化，调用方才能直接比较。
                eventType &= SHCNE_ALLEVENTS;

                // 锁内只做最廉价的工作：拷贝 PIDL。下游要做 SHGetRealIDL、读属性、
                // 取图标等重活，放在锁内会长时间占住 shell 的通知管线，导致通知
                // 堆积甚至丢失。
                LPITEMIDLIST pidl1 = (args && eventHasItem1Pidl(eventType) && args[0]) ? ILClone(args[0]) : nullptr;
                LPITEMIDLIST pidl2 = (args && eventHasItem2Pidl(eventType) && args[1]) ? ILClone(args[1]) : nullptr;

                SHChangeNotification_Unlock(lock);

                // 同线程直连，pidl1/pidl2 只在本次 emit 期间有效，接收方要保留需自行克隆。
                emit watcher->changeNotify(eventType, pidl1, pidl2);

                if (pidl1) ILFree(pidl1);
                if (pidl2) ILFree(pidl2);
            }
        }

        return 0;
    }

    return DefWindowProc(window, message, wparam, lparam);
}
