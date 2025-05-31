#include <cstdio>

#include "../main.h"
#include "TrayIcon.h"

#define TRAY_ICON_UID 1001

NOTIFYICONDATA nid;
HICON hIcon;

void AddTrayIcon(HWND hwnd)
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wchar_t exeDir[MAX_PATH];
    wcscpy_s(exeDir, exePath);
    for (int i = wcslen(exeDir) - 1; i >= 0; --i)
    {
        if (exeDir[i] == L'\\' || exeDir[i] == L'/')
        {
            exeDir[i] = 0;
            break;
        }
    }
    wchar_t iconPath[MAX_PATH];
    swprintf_s(iconPath, L"%s\\assets\\img\\icon.ico", exeDir);
    HICON loadedIcon = (HICON)LoadImageW(nullptr, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (loadedIcon)
    {
        hIcon = loadedIcon;
    }
    else
    {
        hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    }
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, L"Octos2");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hIcon)
    {
        DestroyIcon(hIcon);
        hIcon = nullptr;
    }
}

void ShowTrayMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, L"Open Octos");
    AppendMenu(hMenu, MF_STRING, 2, L"Hide Octos");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, 3, L"Quit");
    SetForegroundWindow(hwnd);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
    if (cmd == 1)
    {
        ShowWindow(app_hwnd, SW_SHOW);
    }
    else if (cmd == 2)
    {
        ShowWindow(app_hwnd, SW_HIDE);
    }
    else if (cmd == 3)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
}