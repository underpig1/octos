#include <windows.h>

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
        *(HWND*)lParam = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
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
    RECT rc;
    GetWindowRect(progman, &rc);
    if (workerw) // windows 10 method
    {
        SetParent(hwnd, workerw);
        SetWindowPos(hwnd, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    else // windows 11 method
    {
        SetParent(hwnd, progman);
        HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
        SetWindowPos(hwnd, shellView, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
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

void CALLBACK DestroyMonitorProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (event == EVENT_OBJECT_DESTROY)
    {
        wchar_t className[256];
        if (GetClassName(hwnd, className, sizeof(className) / sizeof(wchar_t)) &&
            wcscmp(className, L"WorkerW") == 0)
        {
            HWND custom_hwnd = FindWindow(CLASS_NAME, NULL);
            if (!custom_hwnd)
            {
                HWND progman = FindWindow(L"Progman", NULL);
                custom_hwnd = FindWindowEx(progman, NULL, CLASS_NAME, NULL);
            }
            if (!custom_hwnd)
            {
                custom_hwnd = CreateTestWindow();
            }
            AttachWindow(custom_hwnd);
        }
    }
}

void CALLBACK ZOrderMonitorProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                                LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) // windows 11 only
{
    HWND progman = FindWindow(L"Progman", NULL);
    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    HWND custom = FindWindowEx(progman, NULL, CLASS_NAME, NULL);

    if (custom && shellView)
    {
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDPIAware();

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    g_hInstance = hInstance;

    HWND hwnd = CreateTestWindow();
    AttachWindow(hwnd);

    SetWinEventHook(
        EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY,
        NULL,
        DestroyMonitorProc,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
        NULL,
        ZOrderMonitorProc,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}