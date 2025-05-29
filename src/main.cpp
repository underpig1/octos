#include "Core/Core.h"
#include "TrayIcon/TrayIcon.h"
#include "Watchdog/Watchdog.h"
#include "WebView/WebView.h"
#include "main.h"

const std::wstring defaultHtmlPath = L"assets/index.html";
const wchar_t CLASS_NAME[] = L"OctosWorker";
HINSTANCE g_hInstance;
HWND app_hwnd;

std::vector<MonitorWindow> ms;
std::vector<HMONITOR> g_monitors;

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
        CoUninitialize();
        return 0;
    case WM_DESTROY:
    {
        KillTimer(hwnd, 1);
        delete data;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    SetProcessDPIAware();
    InitializeWebViewEnvironment();

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

    InitializeWallpaperWindows();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}