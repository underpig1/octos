#include <wrl.h>
#include <shellscalingapi.h>
#include <shlwapi.h>
#include <dwmapi.h>
#include <corecrt_wstdio.h>
#include <stdio.h>
#include <vector>
#include <thread>
#include <string>
#include <wil/com.h>
#include <WebView2.h>

#define WM_RECREATEHWND (WM_USER + 1)
#define WM_DESTROYTRIGGER (WM_USER + 2)
#define WM_TRAYICON (WM_USER + 3)
#define TRAY_ICON_UID 1001

using namespace Microsoft::WRL;

RECT GetMonitorRect(HMONITOR hMon);
void InitializeWebView(HWND hwnd, const std::wstring &htmlRelativePath);

const std::wstring defaultHtmlPath = L"assets/index.html";
const wchar_t CLASS_NAME[] = L"OctosWorker";
HINSTANCE g_hInstance;
HWND app_hwnd;
NOTIFYICONDATA nid = {};
HICON hIcon = nullptr;

struct WebViewData
{
    wil::com_ptr<ICoreWebView2> webview;
    wil::com_ptr<ICoreWebView2Controller> controller;
};

struct MonitorWindow
{
    HWND hwnd;
    HMONITOR monitor;
    void ExpandToMonitor()
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
    std::wstring htmlPath = defaultHtmlPath;
    ICoreWebView2 *webview;
    ICoreWebView2Controller *controller;
    bool fixing = false;
};

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

HWND CreateWallpaperWindow(const std::wstring &htmlRelativePath)
{
    WebViewData *data = new WebViewData();
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        NULL,
        WS_POPUP | WS_VISIBLE,
        0, 0, 0, 0,
        NULL,
        NULL, g_hInstance, reinterpret_cast<LPVOID>(new WebViewData()));
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    InitializeWebView(hwnd, htmlRelativePath);
    return hwnd;
}

HWND CreateMainWindow()
{
    HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW, CLASS_NAME, L"Octos", WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, g_hInstance, reinterpret_cast<LPVOID>(new WebViewData()));
    InitializeWebView(hwnd, L"assets/index.html");
    return hwnd;
}

void InitializeWebView(HWND hwnd, const std::wstring &htmlRelativePath)
{
    ICoreWebView2 *webview = nullptr;
    ICoreWebView2Controller *controller = nullptr;

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring userDataFolder = tempPath;
    userDataFolder += L"WebView2UserData";

    CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd, htmlRelativePath, &webview, &controller](HRESULT result, ICoreWebView2Environment *env) -> HRESULT
            {
                if (FAILED(result))
                    return result;
                
                env->CreateCoreWebView2Controller(hwnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd, htmlRelativePath, &webview, &controller](HRESULT result, ICoreWebView2Controller *ctrl) -> HRESULT
                        {
                            if (FAILED(result))
                                return result;
                            
                            controller = ctrl;
                            controller->get_CoreWebView2(&webview);

                            WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                            if (!data)
                                return E_FAIL;
                            data->controller = controller;
                            data->webview = webview;

                            // resize
                            RECT bounds;
                            GetClientRect(hwnd, &bounds);
                            controller->put_Bounds(bounds);

                            // set background to transparent
                            Microsoft::WRL::ComPtr<ICoreWebView2Controller2> controller2;
                            if (SUCCEEDED(controller->QueryInterface(IID_PPV_ARGS(&controller2))))
                            {
                                controller2->put_DefaultBackgroundColor({0, 0, 0, 0});
                            }

                            // navigate to local HTML file
                            wchar_t exePath[MAX_PATH];
                            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                            PathRemoveFileSpecW(exePath);
                            std::wstring htmlPath = std::wstring(exePath) + L"\\" + htmlRelativePath;
                            for (auto &c : htmlPath)
                                if (c == L'\\')
                                    c = L'/';
                            std::wstring url = L"file:///" + htmlPath;
                            webview->Navigate(url.c_str());

                            // handle messages
                            webview->add_WebMessageReceived(
                                Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [hwnd](ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT
                                    {
                                        wil::unique_cotaskmem_string messageRaw;
                                        if (SUCCEEDED(args->get_WebMessageAsJson(&messageRaw)))
                                        {
                                            std::wstring msg = messageRaw.get();
                                            if (msg.find(L"\"type\":\"drag\"") != std::wstring::npos)
                                            {
                                                ReleaseCapture();
                                                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                                            }
                                        }
                                        return S_OK;
                                    })
                                    .Get(),
                                nullptr);

                            return S_OK;
                        })
                        .Get());

                return S_OK;
            })
            .Get());
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

void AddTrayIcon(HWND hwnd)
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wchar_t exeDir[MAX_PATH];
    wcscpy_s(exeDir, exePath);
    for (int i = wcslen(exeDir) - 1; i >= 0; --i)
    {
        if (exeDir[i] == L'\\' || exeDir[i] == L'/')
        {
            exeDir[i] = 0;
            break;
        }
    }
    wchar_t iconPath[MAX_PATH];
    swprintf_s(iconPath, L"%s\\assets\\img\\icon.ico", exeDir);
    HICON loadedIcon = (HICON)LoadImageW(nullptr, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (loadedIcon)
    {
        hIcon = loadedIcon;
    }
    else
    {
        hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    }
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, L"Octos2");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hIcon)
    {
        DestroyIcon(hIcon);
        hIcon = nullptr;
    }
}

void ShowTrayMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, L"Open Octos");
    AppendMenu(hMenu, MF_STRING, 2, L"Hide Octos");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, 3, L"Quit");
    SetForegroundWindow(hwnd);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
    if (cmd == 1)
    {
        ShowWindow(app_hwnd, SW_SHOW);
    }
    else if (cmd == 2)
    {
        ShowWindow(app_hwnd, SW_HIDE);
    }
    else if (cmd == 3)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
        WebViewData *data = reinterpret_cast<WebViewData *>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    }

    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_SIZE:
        if (data && data->controller)
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            RECT bounds = {0, 0, width, height};
            data->controller->put_Bounds(bounds);
        }
        return 0;
    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        RemoveTrayIcon();
        PostQuitMessage(0);
        return 0;
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
    case WM_RECREATEHWND:
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
        return 0;
    }
    case WM_DESTROYTRIGGER:
    {
        HWND hwnd = reinterpret_cast<HWND>(lParam);
        if (IsWindow(hwnd))
        {
            DestroyWindow(hwnd);
            wprintf(L"[WndProc] Destroyed window %p safely\n", hwnd);
        }
        return 0;
    }
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
        {
            ShowTrayMenu(hwnd);
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void InitializeWallpaperWindows()
{
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL
                        {
        std::vector<MonitorWindow> *ms = reinterpret_cast<std::vector<MonitorWindow> *>(lParam);
        HWND hwnd = CreateWallpaperWindow(L"assets/index.html");
        AttachWindow(hwnd);
        MonitorWindow mw = MonitorWindow{hwnd, hMon};
        mw.ExpandToMonitor();
        ms->push_back(mw);
        return TRUE; }, reinterpret_cast<LPARAM>(&ms));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDPIAware();
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    g_hInstance = hInstance;

    app_hwnd = CreateMainWindow();
    SetTimer(app_hwnd, 1, 100, NULL);

    HWND trayHwnd = CreateWindowEx(0, CLASS_NAME, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
    AddTrayIcon(trayHwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}