#include "Core.h"
#include "../main.h"
#include "../WebView/WebView.h"
#include "../Storage/Storage.h"
#include "../TrayIcon/TrayIcon.h"

#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

std::vector<MonitorWindow> ms;
std::vector<HMONITOR> g_monitors;
const std::wstring defaultHtmlPath = L"";

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
    // SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    wprintf(L"\n\n!!!! MAKING NEW WINDOW WITH PATH %ws\n\n", htmlPath.c_str());
    if (!htmlPath.empty())
    {
        ShowWindow(hwnd, SW_SHOW);
        AttachWebViewCompositionController(hwnd, htmlPath);
    }
    else // unset
    {
        // LONG_PTR ex = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        // SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
        // SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        //              SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        // BOOL ok = SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        // LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        // style &= ~WS_VISIBLE;
        // SetWindowLongPtr(hwnd, GWL_STYLE, style);
        // SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
        //              SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        // HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
        // SetWindowRgn(hwnd, hrgn, TRUE);
        // SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        wprintf(L"\n\n??? I WASNT ATTACHED\n\n");
        ShowWindow(hwnd, SW_HIDE);
    }
    UpdateWindow(hwnd);
    return hwnd;
}

void RecreateWallpapers()
{
    // for (auto &mw : ms)
    // {
    //     wprintf(L"\nrecreating window with path %ws\n\n", mw.htmlPath.c_str());
    //     if (IsWindow(mw.hwnd))
    //         PostMessage(app_hwnd, WM_USER + 2, 0, reinterpret_cast<LPARAM>(mw.hwnd));
    // }
    // ms.clear();
    // InitializeWallpaperWindows();

    for (auto &mw : ms)
    {
        if (IsWindow(mw.hwnd))
            PostMessage(app_hwnd, WM_USER + 1, 0, reinterpret_cast<LPARAM>(&mw));
    }
}

void ReloadAllWindows()
{
    for (auto &mw : ms)
    {
        if (IsWindow(mw.hwnd))
            ReloadWebViewWindow(mw.hwnd);
    }
}

void CreateMonitorWindow(HMONITOR hMon)
{
    HWND hwnd = CreateWallpaperWindow(defaultHtmlPath);
    if (!hwnd) return;
    AttachWindow(hwnd);
    MonitorWindow mw = {hwnd, hMon};
    mw.ExpandToMonitor();
    ms.push_back(mw);
}

void InitializeWallpaperWindows()
{
    std::lock_guard<std::mutex> lock(all_hwnd_mutex);
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL
                        {
        CreateMonitorWindow(hMon);
        return TRUE; }, NULL);
    all_hwnd_cv.notify_all();
}

void WaitForMainWindowAndCallback(std::function<void()> callback)
{
    if (!IsWindow(app_hwnd))
        std::thread([callback]()
                    {
            std::unique_lock<std::mutex> lock(app_hwnd_mutex);
            bool ready = app_hwnd_cv.wait_for(lock, std::chrono::seconds(2), []()
                                              { return app_hwnd != nullptr && IsWindow(app_hwnd); });
            if (ready)
            {
                callback();
            } })
            .detach();
    else
        callback();
}

void WaitForWallpaperWindowsAndCallback(std::function<void()> callback)
{
    if (ms.empty())
        std::thread([callback]()
                    {
            std::unique_lock<std::mutex> lock(all_hwnd_mutex);
            bool ready = all_hwnd_cv.wait_for(lock, std::chrono::seconds(2), []()
                                              { return !ms.empty(); });
            if (ready)
            {
                callback();
            } })
            .detach();
    else
        callback();
}

HWND CreateMainWindow()
{
    WebViewData *data = new WebViewData();
    const int clientWidth = 850;
    const int clientHeight = 500;
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Octos", WS_THICKFRAME | WS_BORDER, CW_USEDEFAULT, CW_USEDEFAULT, clientWidth, clientHeight, nullptr, nullptr, g_hInstance, reinterpret_cast<LPVOID>(data));
    UINT dpi = GetDpiForWindow(hwnd);
    float scale = dpi / 96.0f;
    RECT rect = {0, 0, static_cast<LONG>(clientWidth * scale), static_cast<LONG>(clientHeight * scale)};
    AdjustWindowRectEx(&rect, WS_THICKFRAME | WS_BORDER, FALSE, 0);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    SetWindowPos(hwnd, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
    UpdateWindow(hwnd);
    // AttachWebViewController(hwnd, L"app/index.html");
    g_appHwndAttached = false;
    SetTimer(hwnd, 1, 100, NULL);
    std::lock_guard<std::mutex> lock(app_hwnd_mutex);
    app_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    app_hwnd_cv.notify_all();
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
        if (!hwnd) return;
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

std::wstring GetDeviceStringFromSzDevice(const std::wstring &szDevice)
{
    DISPLAY_DEVICEW dd;
    dd.cb = sizeof(dd);
    if (EnumDisplayDevicesW(szDevice.c_str(), 0, &dd, 0))
    {
        return dd.DeviceString;
    }
    return szDevice;
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

std::vector<std::wstring> GetSiblingMonitorIds(HWND hwnd)
{
    MonitorWindow thisMw;
    for (auto &mw : ms)
    {
        if (mw.hwnd == hwnd)
            thisMw = mw;
    }
    std::vector<std::wstring> siblingIds;
    for (auto &mw : ms)
    {
        if (mw.htmlPath == thisMw.htmlPath && mw.hwnd != hwnd)
        {
            MONITORINFOEXW mi = {};
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfoW(mw.monitor, &mi))
                siblingIds.push_back(mi.szDevice);
        }
    }
    return siblingIds;
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

std::wstring FindMonitorIdByHwnd(HWND hwnd)
{
    for (auto &mw : ms)
    {
        if (mw.hwnd == hwnd)
        {
            MONITORINFOEXW mi = {};
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfoW(mw.monitor, &mi))
            {
                return mi.szDevice;
            }
        }
    }
    return L"";
}

void NavigateWallpaperByMonitorId(std::wstring monitorId, std::wstring url)
{
    wprintf(L"\n\nnavigating to url %ws\n", url.c_str());
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
    ShowTrayNotification(L"Octos minimized to tray", L"Octos is still running in the background. Click the tray icon > open to re-open the app.");
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

void RestoreMainWindow()
{
    if (IsIconic(app_hwnd))
    {
        WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(app_hwnd, GWLP_USERDATA));
        if (data && data->controller)
        {
            data->controller->put_IsVisible(TRUE);
            ShowWindow(app_hwnd, SW_RESTORE);
        }
    }
    else
    {
        ReattachMainWindow();
        SetForegroundWindow(app_hwnd);
        SetWindowPos(app_hwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }
}

void MinimizeMainWindow()
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(app_hwnd, GWLP_USERDATA));
    if (data && data->controller)
    {
        data->controller->put_IsVisible(FALSE);
        ShowWindow(app_hwnd, SW_MINIMIZE);
    }
}

void OnMainWindowRestore()
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(app_hwnd, GWLP_USERDATA));
    if (data && data->controller)
    {
        data->controller->put_IsVisible(TRUE);
        ShowWindow(app_hwnd, SW_RESTORE);
    }
}