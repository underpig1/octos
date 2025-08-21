#include <thread>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "Watchdog.h"
#include "../main.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"

bool CheckIfExpanded(HWND hwnd, HMONITOR monitor)
{
    RECT rc;
    if (GetWindowRect(hwnd, &rc))
    {
        RECT mrc = GetMonitorRect(monitor);
        if (rc.left != mrc.left ||
            rc.top != mrc.top ||
            rc.right - 1 != mrc.right ||
            rc.bottom != mrc.bottom)
        {
            wprintf(L"[Watchdog] Window rect differs from monitor rect, expanding window\n");
            wprintf(L"Monitor l%d r%d t%d b%d\n", (int)mrc.left, (int)mrc.right, (int)mrc.top, (int)mrc.bottom);
            wprintf(L"Client l%d r%d t%d b%d\n", (int)rc.left, (int)rc.right, (int)rc.top, (int)rc.bottom);
            return false;
        }
    }
    return true;
}

void FixRenderingTarget(MonitorWindow &mw)
{
    MONITORINFO mi = {sizeof(mi)};
    if (!GetMonitorInfo(mw.monitor, &mi))
        return;
    RECT fullMonitorRect = mi.rcMonitor;
    RECT workMonitorRect = mi.rcWork;

    RECT targetRect;
    if (!GetWindowRect(mw.hwnd, &targetRect))
        return;

    struct EnumContext
    {
        RECT fullMonitorRect;
        RECT workMonitorRect;
        RECT targetRect;
        bool fullscreen;
    } ctx{fullMonitorRect, workMonitorRect, targetRect, false};

    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL
        {
            if (!IsWindowVisible(hwnd))
                return TRUE;
            EnumContext *ctx = reinterpret_cast<EnumContext *>(lParam);
            RECT fullMonitorRect = ctx->fullMonitorRect;
            RECT workMonitorRect = ctx->workMonitorRect;
            RECT targetRect = ctx->targetRect;
            RECT windowRect;
            if (GetWindowRect(hwnd, &windowRect))
            {
                bool rectMatch = windowRect.left == targetRect.left &&
                                 windowRect.top == targetRect.top &&
                                 windowRect.right == targetRect.right &&
                                 windowRect.bottom == targetRect.bottom;
                if (rectMatch)
                {
                    wchar_t className[256];
                    GetClassName(hwnd, className, _countof(className));
                    const wchar_t prefix[] = L"Chrome_WidgetWin_";
                    size_t prefixLen = wcslen(prefix);
                    if (wcsncmp(className, prefix, prefixLen) == 0)
                    {
                        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                        if ((exStyle & WS_EX_LAYERED) == 0 || (exStyle & WS_EX_TRANSPARENT) == 0)
                            SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
                        return TRUE;
                    }
                }

                const int tolerance = 2;
                if (windowRect.left <= workMonitorRect.left + tolerance &&
                    windowRect.top <= workMonitorRect.top + tolerance &&
                    windowRect.right >= workMonitorRect.right - tolerance &&
                    windowRect.bottom >= workMonitorRect.bottom - tolerance)
                {
                    if (IsIconic(hwnd))
                        return TRUE;
                    if (GetAncestor(hwnd, GA_ROOT) != hwnd)
                        return TRUE;
                    LONG style = GetWindowLong(hwnd, GWL_STYLE);
                    if (!(style & WS_OVERLAPPEDWINDOW))
                        return TRUE;
                    if (GetWindow(hwnd, GW_OWNER) != NULL || !(style & WS_VISIBLE))
                        return TRUE;
                    BOOL isCloaked = FALSE;
                    HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &isCloaked, sizeof(isCloaked));
                    if (SUCCEEDED(hr) && isCloaked)
                        return TRUE;
                    // wchar_t className[256];
                    // GetClassName(hwnd, className, _countof(className));
                    // wprintf(L"[WATCHDOG NEW FEATURE] FOUND WINDOW WITH CLASS NAME %s\n", className);
                    // if (wcsncmp(className, L"Shell_TrayWnd", 13) == 0 ||
                    //     wcsncmp(className, L"Progman", 7) == 0 ||
                    //     wcsncmp(className, L"Windows.UI.Core.CoreWindow", 26) == 0)
                    //     return TRUE;
                    ctx->fullscreen = true;
                }
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&ctx));
    SetWebViewVisibility(mw.hwnd, !ctx.fullscreen);
}

void FixWallpaperOrder(HWND hwnd)
{
    HWND progman = FindWindow(L"Progman", NULL);
    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    HWND curr = GetNextWindow(shellView, GW_HWNDNEXT);
    bool needs_fixing = true;
    while (curr != nullptr)
    {
        if (curr == hwnd)
        {
            needs_fixing = false;
            break;
        }
        curr = GetNextWindow(curr, GW_HWNDNEXT);
    }
    if (needs_fixing)
    {
        wprintf(L"[Watchdog] Fixing order\n");
        SetWindowPos(hwnd, shellView, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
    }
}

void WatchdogProc()
{
    HWND progman = FindWindow(L"Progman", NULL);

    g_monitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL
                        {
            auto& mos = *reinterpret_cast<std::vector<HMONITOR>*>(lParam);
            mos.push_back(hMon);
            return TRUE; }, reinterpret_cast<LPARAM>(&g_monitors));
    // wprintf(L"[Watchdog] Monitor size %d, window size %d\n", g_monitors.size(), ms.size());

    // check if a new monitor was added
    if (g_monitors.size() > ms.size())
    {
        for (size_t i = ms.size(); i < g_monitors.size(); ++i)
        {
            HMONITOR hMon = g_monitors[i];
            CreateMonitorWindow(hMon);
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
        if (mw.htmlPath.empty())
        {
            if (IsWindowVisible(mw.hwnd))
                ShowWindow(mw.hwnd, SW_HIDE);
            continue;
        }
        // fix rendering target
        FixRenderingTarget(mw);
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
            FixWallpaperOrder(mw.hwnd);
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
            if (!CheckIfExpanded(mw.hwnd, g_monitors[i]))
            {
                mw.ExpandToMonitor();
                if (!CheckIfExpanded(mw.hwnd, g_monitors[i]))
                {
                    mw.fixing = true;
                    PostMessage(app_hwnd, WM_USER + 1, 0, reinterpret_cast<LPARAM>(&mw));
                }
            }
        }
    }
}