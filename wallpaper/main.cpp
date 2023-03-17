#define UNICODE 1

#include <windows.h>
#include <iostream>
#include "./main.h"

HWND workerw = NULL;

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

void Attach(unsigned char *handleBuffer)
{
    LONG_PTR handle = *reinterpret_cast<LONG_PTR *>(handleBuffer);
    HWND hwnd = (HWND)(LONG_PTR)handle;

    HWND progman = FindWindow(L"Progman", NULL);
    LRESULT result = SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);
    
    EnumWindows(&FindWorkerW, reinterpret_cast<LPARAM>(&workerw));
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED);
    SetParent(hwnd, workerw);
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