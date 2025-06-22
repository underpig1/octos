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

void CreateMonitorWindow(HMONITOR hMon)
{
    HWND hwnd = CreateWallpaperWindow(L"wallpapers/Octos/index.html");
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

std::wstring GetMonitorIdsAsJsonString()
{
    std::wstring jsonString = L"{\"type\":\"monitor-ids\",\"data\":[";
    std::vector<std::wstring> monitorIds = GetMonitorIds();
    for (std::wstring monitorId : monitorIds)
        jsonString.append(L"\"" + monitorId + L"\",");
    jsonString.pop_back();
    return jsonString + L"]}";
}

MonitorWindow *FindMonitorWindowById(const std::wstring monitorId)
{
    for (auto &mw : ms)
    {
        MONITORINFOEXW mi = {};
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(mw.monitor, &mi))
        {
            if (monitorId == mi.szDevice)
                return &mw;
        }
    }
    return nullptr;
}

void NavigateWallpaperByMonitorId(std::wstring monitorId, std::wstring url)
{
    MonitorWindow *mw = FindMonitorWindowById(monitorId);
    NavigateWindow(mw->hwnd, url);
}