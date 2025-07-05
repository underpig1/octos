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

struct CallbackData
{
    HWND progman;
    HWND shellView;
    HINSTANCE hInstance;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT lprcMonitor, LPARAM lParam)
{
    CallbackData *data = (CallbackData *)lParam;

    int width = lprcMonitor->right - lprcMonitor->left;
    int height = lprcMonitor->bottom - lprcMonitor->top;

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        L"ChillWallpaperWindow",
        NULL,
        WS_POPUP | WS_VISIBLE,
        lprcMonitor->left, lprcMonitor->top,
        width, height,
        NULL,
        NULL,
        data->hInstance,
        NULL);

    if (!hwnd)
        return FALSE; // Stop enumeration on failure

    SetParent(hwnd, data->progman);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

    SetWindowPos(hwnd, data->shellView,
                 lprcMonitor->left, lprcMonitor->top,
                 width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);

    return TRUE; // Continue enumeration
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDPIAware();

    const wchar_t CLASS_NAME[] = L"ChillWallpaperWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClass(&wc))
        return -1;

    HWND progman = FindWindow(L"Progman", NULL);
    if (!progman)
        return -1;

    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    if (!shellView)
        return -1;

    CallbackData data = {progman, shellView, hInstance};

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&data);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}