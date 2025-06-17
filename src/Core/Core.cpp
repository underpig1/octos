#include "Core.h"
#include "../main.h"
#include "../WebView/WebView.h"

std::vector<MonitorWindow> ms;
std::vector<HMONITOR> g_monitors;

RECT GetMonitorRect(HMONITOR hMon)
{
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMon, &mi))
        return mi.rcMonitor;
    return RECT{0, 0, 0, 0};
}

void MonitorWindow::ExpandToMonitor()
{
    if (IsWindow(hwnd) && monitor)
    {
        RECT monitorRect = GetMonitorRect(monitor);
        HWND parent = GetAncestor(hwnd, GA_PARENT);
        if (parent)
        {
            RECT parentRect;
            GetWindowRect(parent, &parentRect);
            int x = monitorRect.left - parentRect.left;
            int y = monitorRect.top - parentRect.top;
            int w = monitorRect.right - monitorRect.left;
            int h = monitorRect.bottom - monitorRect.top;
            SetWindowPos(hwnd, nullptr, x, y, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
        }
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

HWND CreateWallpaperWindow(const std::wstring &htmlRelativePath)
{
    WebViewData *data = new WebViewData();
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        CLASS_NAME,
        NULL,
        WS_POPUP | WS_VISIBLE,
        0, 0, 0, 0,
        NULL,
        NULL, g_hInstance, reinterpret_cast<LPVOID>(data));
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    AttachWebViewCompositionController(hwnd, htmlRelativePath);
    return hwnd;
}

void RecreateWallpapers()
{
    for (auto &mw : ms)
    {
        if (IsWindow(mw.hwnd))
            PostMessage(app_hwnd, WM_USER + 2, 0, reinterpret_cast<LPARAM>(mw.hwnd));
    }
    ms.clear();
    InitializeWallpaperWindows();
}

void InitializeWallpaperWindows()
{
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL
                        {
        std::vector<MonitorWindow> *ms = reinterpret_cast<std::vector<MonitorWindow> *>(lParam);
        HWND hwnd = CreateWallpaperWindow(L"wallpapers/Octos/index.html");
        AttachWindow(hwnd);
        MonitorWindow mw = MonitorWindow{hwnd, hMon};
        mw.ExpandToMonitor();
        ms->push_back(mw);
        return TRUE; }, reinterpret_cast<LPARAM>(&ms));
}

HWND CreateMainWindow()
{
    WebViewData *data = new WebViewData();
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Octos", WS_THICKFRAME | WS_BORDER, CW_USEDEFAULT, CW_USEDEFAULT, 900, 600, nullptr, nullptr, g_hInstance, reinterpret_cast<LPVOID>(data));
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    // AttachWebViewController(hwnd, L"app/index.html");
    SetTimer(hwnd, 1, 100, NULL);
    return hwnd;
}

void RecreateWindow(LPARAM lParam)
{
    MonitorWindow *pmw = reinterpret_cast<MonitorWindow *>(lParam);
    if (pmw)
    {
        HWND hwnd = CreateWallpaperWindow(pmw->htmlPath);
        AttachWindow(hwnd);
        pmw->hwnd = hwnd;
        pmw->ExpandToMonitor();
        pmw->fixing = false;
        wprintf(L"[Watchdog] Recreated %p\n", IsWindow(pmw->hwnd) ? "true" : "false");
    }
}