#include <windows.h>
#include <iostream>

// Helper: Find the WorkerW window that hosts SHELLDLL_DefView (desktop icons)
HWND GetWorkerWForDesktopIcons()
{
    HWND progman = FindWindow(L"Progman", NULL);
    if (!progman)
        return NULL;

    // First, try to find SHELLDLL_DefView as a child of Progman (classic case)
    HWND shellViewWin = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    if (shellViewWin)
    {
        // No WorkerW if SHELLDLL_DefView is direct child of Progman
        return progman;
    }

    // Otherwise, enumerate all top-level windows to find SHELLDLL_DefView under a WorkerW
    HWND workerw = NULL;
    EnumWindows([](HWND topHandle, LPARAM lParam) -> BOOL
                {
                    HWND shellViewWin = FindWindowEx(topHandle, NULL, L"SHELLDLL_DefView", NULL);
                    if (shellViewWin != NULL)
                    {
                        HWND *pWorkerw = (HWND *)lParam;
                        *pWorkerw = topHandle;
                        return FALSE; // stop enumeration
                    }
                    return TRUE; // continue
                },
                (LPARAM)&workerw);

    return workerw; // might be NULL if not found
}

// Helper: Find the SHELLDLL_DefView window itself
HWND GetShellDllDefView()
{
    HWND progman = FindWindow(L"Progman", NULL);
    if (!progman)
        return NULL;

    // Try to find SHELLDLL_DefView as a child of Progman (classic case)
    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    if (shellView)
        return shellView;

    // Otherwise, enumerate all top-level windows to find SHELLDLL_DefView child under WorkerW
    HWND shellDll = NULL;
    EnumWindows([](HWND topHandle, LPARAM lParam) -> BOOL
                {
                    HWND shellView = FindWindowEx(topHandle, NULL, L"SHELLDLL_DefView", NULL);
                    if (shellView != NULL)
                    {
                        HWND *pShellDll = (HWND *)lParam;
                        *pShellDll = shellView;
                        return FALSE; // stop enumeration
                    }
                    return TRUE; // continue enumeration
                },
                (LPARAM)&shellDll);

    return shellDll; // might be NULL if not found
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
        FillRect(hdc, &rc, redBrush);
        DeleteObject(redBrush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    const wchar_t CLASS_NAME[] = L"ChillWallpaperClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, L"RegisterClass failed!", L"Error", MB_OK);
        return -1;
    }

    // Find WorkerW and SHELLDLL_DefView windows
    HWND workerw = GetWorkerWForDesktopIcons();
    HWND shelldll = GetShellDllDefView();

    if (!workerw || !shelldll)
    {
        MessageBox(NULL, L"Failed to find necessary windows!", L"Error", MB_OK);
        return -1;
    }

    // Get WorkerW window size (to place overlay exactly)
    RECT rc;
    GetWindowRect(workerw, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        L"Chill Wallpaper Window",
        WS_POPUP,
        rc.left, rc.top, width, height,
        NULL, NULL, hInst, NULL);

    if (!hwnd)
    {
        MessageBox(NULL, L"Failed to create window!", L"Error", MB_OK);
        return -1;
    }

    // Parent your window to WorkerW (not to SHELLDLL_DefView)
    SetParent(hwnd, workerw);

    // Set fully opaque layered window (alpha = 255)
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

    // Place your window BELOW the SHELLDLL_DefView (desktop icons)
    SetWindowPos(hwnd, shelldll, rc.left, rc.top, width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
