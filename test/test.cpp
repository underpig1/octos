#include <windows.h>
#include <vector>
#include <chrono>

const wchar_t CLASS_NAME[] = L"WallpaperWindow";
HINSTANCE g_hInstance = NULL;
std::vector<HWND> g_customWindows;

// ========================== Window Setup ==========================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, CreateSolidBrush(RGB(0, 255, 0)));
        EndPaint(hwnd, &ps);
        return 0;
    }
    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
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

void AttachWindow(HWND hwnd, const RECT &rc)
{
    HWND progman = FindWindow(L"Progman", NULL);
    HWND workerw = NULL;
    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);
    Sleep(500);
    EnumWindows(&WorkerWProc, reinterpret_cast<LPARAM>(&workerw));

    if (workerw)
    {
        // Windows 10: attach to WorkerW
        SetParent(hwnd, workerw);
        SetWindowPos(hwnd, nullptr, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    else
    {
        // Windows 11: attach directly to Progman
        HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
        SetParent(hwnd, progman);
        SetWindowPos(hwnd, shellView, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

// ========================== Monitor Enum ==========================

BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC, LPRECT lprcMonitor, LPARAM)
{
    HWND hwnd = CreateTestWindow();
    AttachWindow(hwnd, *lprcMonitor);
    g_customWindows.push_back(hwnd);
    return TRUE;
}

// ========================== Hook Listeners ==========================

void CALLBACK DestroyListenerProc(HWINEVENTHOOK, DWORD event, HWND hwnd,
                                  LONG, LONG, DWORD, DWORD)
{
    if (event != EVENT_OBJECT_DESTROY)
        return;

    wchar_t className[256];
    if (!GetClassName(hwnd, className, 256) || wcscmp(className, L"WorkerW") != 0)
        return;

    for (HWND w : g_customWindows)
    {
        if (IsWindow(w))
            DestroyWindow(w);
    }
    g_customWindows.clear();
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}

void CALLBACK ZOrderListenerProc(HWINEVENTHOOK, DWORD event, HWND hwnd,
                                 LONG, LONG, DWORD, DWORD)
{
    HWND progman = FindWindow(L"Progman", NULL);
    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);

    for (HWND custom : g_customWindows)
    {
        if (!IsWindow(custom))
            continue;

        HWND z = GetNextWindow(custom, GW_HWNDPREV);
        if (z && z != shellView)
        {
            RECT rc;
            GetWindowRect(progman, &rc);
            SetWindowPos(custom, shellView, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                         SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
    }
}

// ========================== Entry Point ==========================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDPIAware();

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    g_hInstance = hInstance;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY, NULL,
                    DestroyListenerProc, 0, 0,
                    WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL,
                    ZOrderListenerProc, 0, 0,
                    WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
