#define UNICODE 1

#include <windows.h>
#include <iostream>
#include "./main.h"

HWND workerw = NULL;
HWND hwnd;

BOOL CALLBACK FindWorkerW(HWND hwnd, LPARAM param)
{
    HWND shelldll = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
    if (shelldll)
    {
        workerw = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
        return FALSE;
    }
    return TRUE;
}

// LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
// {
//     MSLLHOOKSTRUCT *msH = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);

//     if (wParam == WM_MOUSEWHEEL) {
//         scrollDelta = static_cast<std::make_signed_t<WORD>>(HIWORD(msH->mouseData)) < 0 ? -1 : 1;
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

void Attach(unsigned char *handleBuffer)
{
    LONG_PTR handle = *reinterpret_cast<LONG_PTR *>(handleBuffer);
    hwnd = (HWND)(LONG_PTR)handle;

    HWND progman = FindWindow(L"Progman", NULL);
    LRESULT result = SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);
    
    EnumWindows(&FindWorkerW, reinterpret_cast<LPARAM>(&workerw));
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED);
    SetParent(hwnd, workerw);

    // HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE);
    // HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProc, hInstance, NULL);
}

void Detach(unsigned char *handleBuffer)
{
    LONG_PTR handle = *reinterpret_cast<LONG_PTR *>(handleBuffer);
    HWND hwnd = (HWND)(LONG_PTR)handle;
    SetParent(hwnd, NULL);
}

POINT MousePosition()
{
    POINT point;
    GetCursorPos(&point);
    return point;
}

BOOL LMousePressed()
{
    return GetAsyncKeyState(VK_LBUTTON) < 0;
}

BOOL MMousePressed()
{
    return GetAsyncKeyState(VK_MBUTTON) < 0;
}

BOOL RMousePressed()
{
    return GetAsyncKeyState(VK_RBUTTON) < 0;
}

BOOL InForeground()
{
    if (workerw == NULL) {
        HWND progman = FindWindow(L"Progman", NULL);
        LRESULT result = SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);
        EnumWindows(&FindWorkerW, (LPARAM)&workerw);
    }

    HWND fg = GetForegroundWindow();
    DWORD fgpid = GetWindowThreadProcessId(fg, NULL);
    DWORD wwpid = GetWindowThreadProcessId(workerw, NULL);
    return fgpid == wwpid;
}

char* ForegroundWindow()
{
    GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    HWND fg = GetForegroundWindow();
    int len = GetWindowTextLength(fg) + 1;
    WCHAR *wstr = new WCHAR[len];
    GetWindowText(fg, wstr, len);
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *wnd_title = new char[size_needed];
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, wnd_title, size_needed, NULL, NULL);
    return wnd_title;
}

bool* KeyboardState()
{
    bool keys[256];
    for (int k = 0; k <= 255; k++) keys[k] = GetAsyncKeyState(k) < 0;
    return keys;
}

void SetTaskbar(BOOL state = TRUE)
{
    HWND hShellWnd = FindWindow(L"Shell_TrayWnd", NULL);
    ShowWindow(hShellWnd, state ? SW_SHOW : SW_HIDE);
    UpdateWindow(hShellWnd);
}

void SendMediaEvent(int state = 0)
{
    SetForegroundWindow(hwnd);

    int key;
    if (state == -1) key = VK_MEDIA_PREV_TRACK;
    else if (state == 0) key = VK_MEDIA_PLAY_PAUSE;
    else if (state == 1) key = VK_MEDIA_NEXT_TRACK;
    else if (state <= 26) key = 0x6E + state;
    else return;

    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = key;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));

    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}