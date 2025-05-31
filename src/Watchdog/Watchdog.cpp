#include <thread>

#include "Watchdog.h"
#include "../main.h"
#include "../Core/Core.h"

void WatchdogProc()
{
    wprintf(L"[Watchdog] Timer\n");

    HWND progman = FindWindow(L"Progman", NULL);

    g_monitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL
                        {
            auto& mos = *reinterpret_cast<std::vector<HMONITOR>*>(lParam);
            mos.push_back(hMon);
            return TRUE; }, reinterpret_cast<LPARAM>(&g_monitors));
    wprintf(L"[Watchdog] Monitor size %d, window size %d\n", g_monitors.size(), ms.size());

    // check if a new monitor was added
    if (g_monitors.size() > ms.size())
    {
        for (size_t i = ms.size(); i < g_monitors.size(); ++i)
        {
            HMONITOR hMon = g_monitors[i];
            HWND hwnd = CreateWallpaperWindow(defaultHtmlPath);
            AttachWindow(hwnd);
            MonitorWindow mw = {hwnd, hMon};
            mw.ExpandToMonitor();
            ms.push_back(mw);
            wprintf(L"[Watchdog] Added new monitor window %p for monitor %zu\n", hwnd, i);
        }
    }

    for (size_t i = 0; i < ms.size(); ++i)
    {
        MonitorWindow &mw = ms[i];
        // check if quietly destroyed
        if (mw.fixing)
            continue;
        if (!IsWindow(mw.hwnd))
        {
            mw.fixing = true;
            wprintf(L"[Watchdog] Window %p no longer exists!\n", mw.hwnd);
            MonitorWindow *pmw = &mw;
            std::thread([pmw]()
                        {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    wprintf(L"[Watchdog] Recreating window... %p \n", pmw->hwnd);
                    PostMessage(app_hwnd, WM_USER + 1, 0, reinterpret_cast<LPARAM>(pmw)); })
                .detach();
            continue;
        }
        // check if not parented
        HWND parent = GetAncestor(mw.hwnd, GA_PARENT);
        if (!parent || parent == GetDesktopWindow())
        {
            wprintf(L"Parent does not exist!\n", mw.hwnd);
            AttachWindow(mw.hwnd);
            mw.ExpandToMonitor();
        }
        // check if zorder needs fixing (win11)
        if (parent == progman)
        {
            HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
            HWND curr = GetNextWindow(shellView, GW_HWNDNEXT);
            bool needs_fixing = true;
            while (curr != nullptr)
            {
                if (curr == mw.hwnd)
                {
                    needs_fixing = false;
                    break;
                }
                curr = GetNextWindow(curr, GW_HWNDNEXT);
            }
            if (needs_fixing)
            {
                wprintf(L"[Watchdog] Fixing order\n");
                SetWindowPos(mw.hwnd, shellView, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
            }
        }
        // check if window isnt assinged to a monitor
        RECT a = GetMonitorRect(mw.monitor);
        if (i + 1 > g_monitors.size())
        {
            wprintf(L"[Watchdog] Window doesn't have monitor, deleting\n");
            if (IsWindow(mw.hwnd))
            {
                PostMessage(app_hwnd, WM_USER + 2, 0, reinterpret_cast<LPARAM>(mw.hwnd));
            }
            ms.erase(ms.begin() + i);
            --i;
            continue;
        }
        // check if change in monitor
        if (mw.monitor != g_monitors[i])
        {
            wprintf(L"[Watchdog] Change in monitor\n");
            mw.monitor = g_monitors[i];
            mw.ExpandToMonitor();
        }
        else // check change in monitor size
        {
            RECT rc;
            if (GetWindowRect(mw.hwnd, &rc))
            {
                RECT mrc = GetMonitorRect(g_monitors[i]);

                if (rc.left != mrc.left ||
                    rc.top != mrc.top ||
                    rc.right != mrc.right ||
                    rc.bottom != mrc.bottom)
                {
                    wprintf(L"[Watchdog] Window rect differs from monitor rect, expanding window\n");
                    wprintf(L"Monitor l%d r%d t%d b%d\n", (int)mrc.left, (int)mrc.right, (int)mrc.top, (int)mrc.bottom);
                    wprintf(L"Client l%d r%d t%d b%d\n", (int)rc.left, (int)rc.right, (int)rc.top, (int)rc.bottom);
                    mw.ExpandToMonitor();
                }
            }
        }
    }
}