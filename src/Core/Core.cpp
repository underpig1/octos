#include "Core.h"
#include "../main.h"
#include "../WebView/WebView.h"
#include "../Storage/Storage.h"

std::vector<MonitorWindow> ms;
std::vector<HMONITOR> g_monitors;
const std::wstring defaultHtmlPath = ResolvePath(L"default/index.html");

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
            int w = monitorRect.right - monitorRect.left + 1; // unique vals
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

HWND CreateWallpaperWindow(const std::wstring &htmlPath)
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
    AttachWebViewCompositionController(hwnd, htmlPath);
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

void CreateMonitorWindow(HMONITOR hMon)
{
    HWND hwnd = CreateWallpaperWindow(defaultHtmlPath);
    AttachWindow(hwnd);
    MonitorWindow mw = {hwnd, hMon};
    mw.ExpandToMonitor();
    ms.push_back(mw);
}

void InitializeWallpaperWindows()
{
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL
                        {
        CreateMonitorWindow(hMon);
        return TRUE; }, NULL);
}

HWND CreateMainWindow()
{
    WebViewData *data = new WebViewData();
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Octos", WS_THICKFRAME | WS_BORDER, CW_USEDEFAULT, CW_USEDEFAULT, 1350, 800, nullptr, nullptr, g_hInstance, reinterpret_cast<LPVOID>(data));
    UpdateWindow(hwnd);
    // AttachWebViewController(hwnd, L"app/index.html");
    g_appHwndAttached = false;
    SetTimer(hwnd, 1, 100, NULL);
    app_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    return hwnd;
}

void RecreateWindow(LPARAM lParam)
{
    MonitorWindow *pmw = reinterpret_cast<MonitorWindow *>(lParam);
    if (pmw)
    {
        if (pmw->hwnd && IsWindow(pmw->hwnd))
        {
            DestroyWindow(pmw->hwnd);
            pmw->hwnd = nullptr;
        }
        wprintf(L"IM GETTING RECREATED WITH NEW URL %ws", pmw->htmlPath.c_str());
        if (pmw->htmlPath.empty())
            pmw->htmlPath = defaultHtmlPath;
        HWND hwnd = CreateWallpaperWindow(pmw->htmlPath);
        AttachWindow(hwnd);
        pmw->hwnd = hwnd;
        pmw->ExpandToMonitor();
        pmw->fixing = false;
        wprintf(L"[Watchdog] Recreated %p\n", IsWindow(pmw->hwnd) ? "true" : "false");
    }
}

void SetWallpaperVisibility(bool visible)
{
    for (auto &mw : ms)
        ShowWindow(mw.hwnd, visible ? SW_SHOW : SW_HIDE);
}

std::vector<std::wstring> GetMonitorIds()
{
    std::vector<std::wstring> monitorIds;
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM data) -> BOOL
                        {
        auto* ids = reinterpret_cast<std::vector<std::wstring>*>(data);
        MONITORINFOEXW infoEx = {};
        infoEx.cbSize = sizeof(infoEx);
        if (GetMonitorInfoW(hMon, &infoEx))
            ids->push_back(infoEx.szDevice);
        return TRUE; }, reinterpret_cast<LPARAM>(&monitorIds));
    return monitorIds;
}

MonitorWindow *FindMonitorWindowById(const std::wstring monitorId)
{
    for (auto &mw : ms)
    {
        MONITORINFOEXW mi = {};
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(mw.monitor, &mi))
        {
            wprintf(L"ITERATING MONITORS %ws\n", mi.szDevice);
            if (monitorId == mi.szDevice)
                return &mw;
        }
    }
    return nullptr;
}

std::vector<std::wstring> FindMonitorIdsByHwnd(HWND hwnd)
{
    std::vector<std::wstring> monitorIds;
    for (auto &mw : ms)
    {
        if (mw.hwnd == hwnd)
        {
            MONITORINFOEXW mi = {};
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfoW(mw.monitor, &mi))
                monitorIds.push_back(mi.szDevice);
        }
    }
    return monitorIds;
}

void NavigateWallpaperByMonitorId(std::wstring monitorId, std::wstring url)
{
    MonitorWindow *mw = FindMonitorWindowById(monitorId);
    if (mw && mw->hwnd && mw->htmlPath != url)
    {
        mw->htmlPath = url;
        PostMessage(app_hwnd, WM_USER + 1, 0, reinterpret_cast<LPARAM>(mw));
        // NavigateWindow(mw->hwnd, url);
    }
}

void NavigateAllWallpapers(std::wstring url)
{
    for (auto &mw : ms)
    {
        if (mw.hwnd && mw.htmlPath != url)
        {
            mw.htmlPath = url;
            PostMessage(app_hwnd, WM_USER + 1, 0, reinterpret_cast<LPARAM>(&mw));
        }
    }
}

void ReleaseMainWindow()
{
    g_appHwndAttached = false;
    HandleOnDestroy(app_hwnd);
    ShowWindow(app_hwnd, SW_HIDE);
}

void ReattachMainWindow()
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(app_hwnd, GWLP_USERDATA));
    if (!data || !data->controller)
    {
        data = new WebViewData();
        SetWindowLongPtr(app_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
        ShowWindow(app_hwnd, SW_SHOW);
    }
}