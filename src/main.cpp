#include "Core/Core.h"
#include "TrayIcon/TrayIcon.h"
#include "Watchdog/Watchdog.h"
#include "WebView/WebView.h"
// #include "Dispatch/Dispatch.h"
#include "Event/Event.h"
#include "main.h"
#include <windowsx.h>

const wchar_t CLASS_NAME[] = L"OctosWorker";
HINSTANCE g_hInstance;
HWND app_hwnd;

void OnClose()
{
    KillTimer(app_hwnd, 1);
    KillTimer(app_hwnd, 2);
    DestroyTrayIcon();
    CoUninitialize();
    UninstallEventHooks();
    PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_NCCALCSIZE:
    {
        if (app_hwnd != hwnd)
            break;
        if (wParam)
        {
            NCCALCSIZE_PARAMS *pParams = (NCCALCSIZE_PARAMS *)lParam;
            const int border = 8;
            pParams->rgrc[0].left += border;
            pParams->rgrc[0].right -= border;
            pParams->rgrc[0].bottom -= border;
            return 0;
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        if (hwnd == app_hwnd)
        {
            auto mmi = reinterpret_cast<MINMAXINFO *>(lParam);
            mmi->ptMinTrackSize.x = 800;
            mmi->ptMinTrackSize.y = 500;
            return 0;
        }
    }
    // case WM_NCHITTEST:
    // {
    //     POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    //     ScreenToClient(hwnd, &pt);
    //     RECT rc;
    //     GetClientRect(hwnd, &rc);
    //     const int BORDER_WIDTH = 8;

    //     if (pt.x < BORDER_WIDTH)
    //     {
    //         if (pt.y < BORDER_WIDTH)
    //             return HTTOPLEFT;
    //         else if (pt.y > rc.bottom - BORDER_WIDTH)
    //             return HTBOTTOMLEFT;
    //         else
    //             return HTLEFT;
    //     }
    //     else if (pt.x > rc.right - BORDER_WIDTH)
    //     {
    //         if (pt.y < BORDER_WIDTH)
    //             return HTTOPRIGHT;
    //         else if (pt.y > rc.bottom - BORDER_WIDTH)
    //             return HTBOTTOMRIGHT;
    //         else
    //             return HTRIGHT;
    //     }
    //     else if (pt.y < BORDER_WIDTH)
    //         return HTTOP;
    //     else if (pt.y > rc.bottom - BORDER_WIDTH)
    //         return HTBOTTOM;
    //     return HTCLIENT;
    // }
    case WM_NCCREATE:
        wprintf(L"[WinMain] Created\n");
        HandleOnCreate(hwnd, lParam);
        return TRUE;
    case WM_SIZE:
        wprintf(L"[WinMain] Resized\n");
        HandleResize(hwnd, lParam);
        return 0;
    case WM_ACTIVATE:
        for (auto &mw : ms)
            FixWallpaperOrder(mw.hwnd);
    case WM_SETFOCUS:
        for (auto &mw : ms)
            FixWallpaperOrder(mw.hwnd);
    case WM_KILLFOCUS:
        for (auto &mw : ms)
            FixWallpaperOrder(mw.hwnd);
    case WM_SHOWWINDOW:
    {
        wprintf(L"[WinMain] Shown\n");
        return 0;
    }
    case WM_CLOSE:
    {
        wprintf(L"[WinMain] Closed\n");
        if (app_hwnd == hwnd)
            ShowWindow(hwnd, SW_HIDE);
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
            ShowTrayMenu();
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
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
    wc.hbrBackground = nullptr;
    RegisterClass(&wc);
    g_hInstance = hInstance;

    InitializeTrayIcon();

    app_hwnd = CreateMainWindow();
    // SetTimer(app_hwnd, 2, 10, NULL);

    InitializeWallpaperWindows();
    InstallEventHooks();
    // std::atexit(OnClose);
    wprintf(L"[WinMain] Initialized");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    OnClose();
    return 0;
}