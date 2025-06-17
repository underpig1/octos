#include <cstdio>

#include "../main.h"
#include "TrayIcon.h"
#include "../Storage/Storage.h"

#define TRAY_ICON_UID 1001

NOTIFYICONDATA nid;
HICON hIcon;
HWND trayHwnd;

void InitializeTrayIcon()
{
    std::wstring iconPath = ResolvePath(L"assets\\img\\icon.ico");
    HICON loadedIcon = (HICON)LoadImageW(nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (loadedIcon)
        hIcon = loadedIcon;
    else
        hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    trayHwnd = CreateWindowEx(0, CLASS_NAME, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, g_hInstance, 0);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = trayHwnd;
    nid.uID = TRAY_ICON_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, L"Octos");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void DestroyTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hIcon)
    {
        DestroyIcon(hIcon);
        hIcon = nullptr;
    }
}

void ShowTrayMenu()
{
    if (!trayHwnd)
        return;

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, L"Open Octos");
    AppendMenu(hMenu, MF_STRING, 2, L"Hide Octos");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, 3, L"Quit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(trayHwnd);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, trayHwnd, NULL);
    PostMessage(trayHwnd, WM_NULL, 0, 0);
    
    DestroyMenu(hMenu);

    if (cmd == 1)
    {
        if (IsIconic(app_hwnd))
            ShowWindow(app_hwnd, SW_RESTORE);
        else
            ShowWindow(app_hwnd, SW_SHOW);
        SetForegroundWindow(app_hwnd);
        SetWindowPos(app_hwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }
    else if (cmd == 2)
        ShowWindow(app_hwnd, SW_HIDE);
    else if (cmd == 3)
        PostMessage(trayHwnd, WM_CLOSE, 0, 0);
}