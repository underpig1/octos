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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    const wchar_t CLASS_NAME[] = L"ChillWallpaperWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND progman = FindWindow(L"Progman", NULL);
    if (!progman)
        return -1;

    SetProcessDPIAware();
    RECT rc;
    GetWindowRect(progman, &rc);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        NULL,
        WS_POPUP | WS_VISIBLE,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        NULL, // initially no parent — WS_POPUP ignores this anyway
        NULL, hInstance, NULL);

    if (!hwnd)
        return -1;

    // Now correctly set Progman as parent (for Z-order + ownership)
    SetParent(hwnd, progman);

    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    // SetParent(hwnd, shellView);
    SetWindowPos(hwnd, shellView, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
