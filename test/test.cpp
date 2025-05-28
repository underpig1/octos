#include <windows.h>
#include <corecrt_wstdio.h>
#include <stdio.h>
#include <vector>
#include <thread>

HWND CreateTestWindow();
void AttachWindow(HWND hwnd);
void WatchdogProc();
BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM lParam);
RECT GetMonitorRect(HMONITOR hMon)
{
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMon, &mi))
        return mi.rcMonitor;
    return RECT{0, 0, 0, 0};
}
struct MonitorWindow
{
    HWND hwnd;
    HMONITOR monitor;
    void ExpandToMonitor()
    {
        if (IsWindow(hwnd))
        {
            RECT rc = GetMonitorRect(monitor);
            SetWindowPos(hwnd, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }
    bool fixing = false;
};
std::vector<MonitorWindow> ms;
std::vector<HMONITOR> g_monitors;
HWND g_main;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, CreateSolidBrush(RGB(0, 255, 0)));
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
    {
        KillTimer(hwnd, 1);
        return 0;
    }
    case WM_TIMER:
    {
        WatchdogProc();
        wprintf(L"[WndProc] Timer\n");
        return 0;
    }
    case WM_USER + 1:
    {
        MonitorWindow *pmw = reinterpret_cast<MonitorWindow *>(lParam);
        if (pmw)
        {
            HWND hwnd = CreateTestWindow();
            AttachWindow(hwnd);
            pmw->hwnd = hwnd;
            pmw->ExpandToMonitor();
            pmw->fixing = false;
            wprintf(L"[Watchdog] Recreated %p\n", IsWindow(pmw->hwnd) ? "true" : "false");
        }
        return 0;
    }
    case WM_USER + 2:
    {
        HWND hwnd = reinterpret_cast<HWND>(lParam);
        if (IsWindow(hwnd))
        {
            DestroyWindow(hwnd);
            wprintf(L"[WndProc] Destroyed window %p safely\n", hwnd);
        }
        return 0;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

BOOL CALLBACK WorkerWProc(HWND hwnd, LPARAM lParam)
{
    HWND shellView = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
    if (shellView)
    {
        *(HWND *)lParam = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
        return FALSE;
    }
    return TRUE;
}

void AttachWindow(HWND hwnd)
{
    HWND progman = FindWindow(L"Progman", NULL);
    HWND workerw = NULL;
    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);
    EnumWindows(&WorkerWProc, reinterpret_cast<LPARAM>(&workerw));
    // RECT rc;
    // GetWindowRect(progman, &rc);
    if (workerw) // windows 10 method
    {
        SetParent(hwnd, workerw);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    }
    else // windows 11 method
    {
        SetParent(hwnd, progman);
        HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
        SetWindowPos(hwnd, shellView, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    }
}

const wchar_t CLASS_NAME[] = L"WallpaperWindow";
HINSTANCE g_hInstance = NULL;

HWND CreateTestWindow()
{
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        NULL,
        WS_POPUP | WS_VISIBLE,
        0, 0, 0, 0,
        NULL,
        NULL, g_hInstance, NULL);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    return hwnd;
}

// void CALLBACK DestroyMonitorProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
//                                  LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
// {
//     if (event != EVENT_OBJECT_DESTROY)
//         return;

//     wchar_t className[256];
//     if (!GetClassName(hwnd, className, _countof(className)))
//         return;

//     if (wcscmp(className, L"WorkerW") != 0)
//         return;

//     wprintf(L"WorkerW Destroyed - Recreating custom window...\n");

//     HWND custom_hwnd = FindWindow(CLASS_NAME, NULL);
//     if (!custom_hwnd)
//     {
//         HWND progman = FindWindow(L"Progman", NULL);
//         if (progman)
//             custom_hwnd = FindWindowEx(progman, NULL, CLASS_NAME, NULL);
//     }
//     if (!custom_hwnd)
//     {
//         custom_hwnd = CreateTestWindow();
//         if (!custom_hwnd)
//         {
//             wprintf(L"Failed to create custom window!\n");
//             return;
//         }
//     }
//     AttachWindow(custom_hwnd);
// }

// void CALLBACK ZOrderMonitorProc(HWINEVENTHOOK, DWORD, HWND hwnd,
//                                 LONG idObject, LONG, DWORD, DWORD)
// {
//     if (idObject != OBJID_WINDOW)
//         return;

//     HWND progman = FindWindow(L"Progman", NULL);
//     if (!progman)
//         return;

//     HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
//     HWND custom = FindWindowEx(progman, NULL, CLASS_NAME, NULL);

//     if (!IsWindow(shellView) || !IsWindow(custom))
//         return;

//     HWND expected = GetNextWindow(shellView, GW_HWNDNEXT);

//     if (custom != expected)
//     {
//         RECT rc;
//         GetWindowRect(progman, &rc);
//         SetWindowPos(custom, shellView,
//                      rc.left, rc.top,
//                      rc.right - rc.left, rc.bottom - rc.top,
//                      SWP_NOACTIVATE | SWP_SHOWWINDOW);
//     }
// }

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
            HWND hwnd = CreateTestWindow();
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
                    PostMessage(g_main, WM_USER + 1, 0, reinterpret_cast<LPARAM>(pmw)); })
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
                PostMessage(g_main, WM_USER + 2, 0, reinterpret_cast<LPARAM>(mw.hwnd));
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

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM lParam)
{
    std::vector<MonitorWindow> *ms = reinterpret_cast<std::vector<MonitorWindow> *>(lParam);
    HWND hwnd = CreateTestWindow();
    AttachWindow(hwnd);
    MonitorWindow mw = MonitorWindow{hwnd, hMon};
    mw.ExpandToMonitor();
    ms->push_back(mw);
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDPIAware();

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    g_hInstance = hInstance;

    // HWND hwnd = CreateTestWindow();
    // ms.push_back({hwnd});
    // AttachWindow(hwnd);
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&ms));

    g_main = CreateTestWindow();
    SetTimer(g_main, 1, 100, NULL);

    // SetWinEventHook(
    //     EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY,
    //     NULL,
    //     DestroyMonitorProc,
    //     0, 0,
    //     WINEVENT_SKIPOWNPROCESS);

    // SetWinEventHook(
    //     EVENT_OBJECT_REORDER, EVENT_OBJECT_REORDER,
    //     NULL,
    //     ZOrderMonitorProc,
    //     0, 0,
    //     WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}