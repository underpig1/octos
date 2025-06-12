#include "Core/Core.h"
#include "TrayIcon/TrayIcon.h"
#include "Watchdog/Watchdog.h"
#include "WebView/WebView.h"
// #include "Dispatch/Dispatch.h"
#include "Event/Event.h"
#include "main.h"

const std::wstring defaultHtmlPath = L"assets/index.html";
const wchar_t CLASS_NAME[] = L"OctosWorker";
HINSTANCE g_hInstance;
HWND app_hwnd;

void OnClose()
{
    KillTimer(app_hwnd, 1);
    KillTimer(app_hwnd, 2);
    RemoveTrayIcon();
    CoUninitialize();
    UninstallEventHooks();
    PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_NCCREATE:
        wprintf(L"[WinMain] Created\n");
        HandleOnCreate(hwnd, lParam);
        return TRUE;
    case WM_SIZE:
        wprintf(L"[WinMain] Resized\n");
        HandleResize(hwnd, lParam);
        return 0;
    case WM_SHOWWINDOW:
    {
        wprintf(L"[WinMain] Shown\n");
        InstallEventHooks();
        return 0;
    }
    case WM_CLOSE:
    {
        wprintf(L"[WinMain] Closed\n");
        OnClose();
        return 0;
    }
    case WM_DESTROY:
    {
        wprintf(L"[WinMain] Destroyed\n");
        HandleOnDestroy(hwnd);
        return 0;
    }
    case WM_TIMER:
    {
        WatchdogProc();
        return 0;
    }
    case WM_RECREATEHWND:
    {
        wprintf(L"[WinMain] Recreated\n");
        RecreateWindow(lParam);
        return 0;
    }
    case WM_DESTROYTRIGGER:
    {
        wprintf(L"[WinMain] Destroy triggered\n");
        HWND re_hwnd = reinterpret_cast<HWND>(lParam);
        if (IsWindow(re_hwnd))
        {
            DestroyWindow(re_hwnd);
        }
        return 0;
    }
    case WM_TRAYICON:
        wprintf(L"[WinMain] Tray created\n");
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
        {
            ShowTrayMenu(hwnd);
        }
        return 0;
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

    HWND trayHwnd = CreateWindowEx(0, CLASS_NAME, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
    AddTrayIcon(trayHwnd);

    app_hwnd = CreateMainWindow();
    SetTimer(app_hwnd, 1, 100, NULL);
    // SetTimer(app_hwnd, 2, 10, NULL);

    InitializeWallpaperWindows();

    // InstallEventHooks();

    // std::atexit(OnClose);
    wprintf(L"[WinMain] Initialized");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}