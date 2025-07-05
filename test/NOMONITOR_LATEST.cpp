#include <windows.h>
#include <corecrt_wstdio.h>
#include <stdio.h>
#include <list>

void AttachWindow(HWND hwnd);
void WatchdogProc();
std::list<HWND> ws;

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
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_TIMER)
    {
        WatchdogProc();
        wprintf(L"[WndProc] Timer\n");
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

// void CALLBACK DestroyMonitorProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
//                                  LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
// {
//     if (event != EVENT_OBJECT_DESTROY)
//         return;

//     wchar_t className[256];
//     if (!GetClassName(hwnd, className, _countof(className)))
//         return;

//     if (wcscmp(className, L"WorkerW") != 0)
//         return;

//     wprintf(L"WorkerW Destroyed - Recreating custom window...\n");

//     HWND custom_hwnd = FindWindow(CLASS_NAME, NULL);
//     if (!custom_hwnd)
//     {
//         HWND progman = FindWindow(L"Progman", NULL);
//         if (progman)
//             custom_hwnd = FindWindowEx(progman, NULL, CLASS_NAME, NULL);
//     }
//     if (!custom_hwnd)
//     {
//         custom_hwnd = CreateTestWindow();
//         if (!custom_hwnd)
//         {
//             wprintf(L"Failed to create custom window!\n");
//             return;
//         }
//     }
//     AttachWindow(custom_hwnd);
// }

void CALLBACK ZOrderMonitorProc(HWINEVENTHOOK, DWORD, HWND hwnd,
                                LONG idObject, LONG, DWORD, DWORD)
{
    if (idObject != OBJID_WINDOW)
        return;

    HWND progman = FindWindow(L"Progman", NULL);
    if (!progman)
        return;

    HWND shellView = FindWindowEx(progman, NULL, L"SHELLDLL_DefView", NULL);
    HWND custom = FindWindowEx(progman, NULL, CLASS_NAME, NULL);

    if (!IsWindow(shellView) || !IsWindow(custom))
        return;

    HWND expected = GetNextWindow(shellView, GW_HWNDNEXT);

    if (custom != expected)
    {
        RECT rc;
        GetWindowRect(progman, &rc);
        SetWindowPos(custom, shellView,
                     rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

void WatchdogProc()
{
    wprintf(L"[Watchdog] Timer\n");
    for (auto it = ws.begin(); it != ws.end();)
    {
        if (!IsWindow(*it))
        {
            wprintf(L"[Watchdog] Window %p no longer exists!\n", *it);
            Sleep(1000);
            HWND replacement = CreateTestWindow();
            AttachWindow(replacement);
            it = ws.erase(it);
            ws.push_front(replacement);
        }
        else
        {
            wprintf(L"[Watchdog] Window exists\n", *it);
            HWND parent = GetAncestor(*it, GA_PARENT);
            if (!parent || parent == GetDesktopWindow())
            {
                wprintf(L"Parent does not exist!\n", *it);
                AttachWindow(*it);
            }
            ++it;
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDPIAware();

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
    g_hInstance = hInstance;

    HWND hwnd = CreateTestWindow();
    ws.push_back(hwnd);
    AttachWindow(hwnd);

    HWND main = CreateTestWindow();
    SetTimer(main, 1, 100, NULL);

    // SetWinEventHook(
    //     EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY,
    //     NULL,
    //     DestroyMonitorProc,
    //     0, 0,
    //     WINEVENT_SKIPOWNPROCESS);

    SetWinEventHook(
        EVENT_OBJECT_REORDER, EVENT_OBJECT_REORDER,
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