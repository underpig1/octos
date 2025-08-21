#include <cstdio>

#include "../main.h"
#include "TrayIcon.h"
#include "../Storage/Storage.h"
#include "../Core/Core.h"
#include "../Bridge/Bridge.h"

#define TRAY_ICON_UID 1001

NOTIFYICONDATA nid;
HWND trayHwnd;

void InitializeTrayIcon()
{
    trayHwnd = CreateWindowEx(0, CLASS_NAME, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, g_hInstance, 0);
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = trayHwnd;
    nid.uID = TRAY_ICON_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = g_hIcon;
    wcscpy_s(nid.szTip, L"Octos");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void DestroyTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowTrayMenu()
{
    if (!trayHwnd)
        return;

    HMENU hMenu = CreatePopupMenu();

    AppendMenu(hMenu, MF_STRING, 1, L"Open");

    if (g_appHwndAttached)
        AppendMenu(hMenu, MF_STRING, 2, L"Minimize to tray");

    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    bool visible = IsWindowVisible(ms[0].hwnd);
    AppendMenu(hMenu, MF_STRING, 3, visible ? L"Pause wallpaper" : L"Resume wallpaper");

    // if (!g_allParams.empty())
    // {
    //     HMENU hSelectorMenu = CreatePopupMenu();
    //     for (size_t i = 0; i < g_allParams.size(); ++i)
    //     {
    //         AppendMenuW(
    //             hSelectorMenu,
    //             MF_STRING,
    //             10 + (UINT)i,
    //             g_allParams[i].name.c_str());
    //     }
    //     AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSelectorMenu, L"Set wallpaper");
    // }
    // AppendMenu(hMenu, MF_STRING, 4, L"Refresh");
    // AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, 5, L"Go to docs");
    AppendMenu(hMenu, MF_STRING, 7, L"Go to GitHub");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, 6, L"Quit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(trayHwnd);
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, trayHwnd, NULL);
    PostMessage(trayHwnd, WM_NULL, 0, 0);

    DestroyMenu(hMenu);

    if (cmd == 1)
    {
        RestoreMainWindow();
    }
    else if (cmd == 2)
        ReleaseMainWindow();
    else if (cmd == 3)
    {
        SetWallpaperVisibility(!visible);
        DispatchVisibility();
    }
    // else if (cmd == 4)
    // {
    //     RecreateWallpapers();
    //     std::wstring message = IterateWallpapersAsJsonString();
    //     DispatchJson(message);
    // }
    else if (cmd == 5)
    {
        ShellExecute(NULL, L"open", L"https://underpig1.github.io/octos/guides/", NULL, NULL, SW_SHOWNORMAL);
    }
    else if (cmd == 6)
        PostMessage(trayHwnd, WM_USER + 4, 0, 0);
    else if (cmd == 7)
    {
        ShellExecute(NULL, L"open", L"https://github.com/underpig1/octos", NULL, NULL, SW_SHOWNORMAL);
    }
    else if (cmd >= 10 && cmd < 10 + (int)g_allParams.size())
    {
        const ConfigParams params = g_allParams[cmd - 10];
        if (!params.entryPath.empty())
        {
            NavigateAllWallpapers(params.entryPath);
            DispatchNavigateAllWallpapers(params.folderPath);
        }
    }
}

void ShowTrayNotification(const wchar_t *title, const wchar_t *message)
{
    if (!GetPref(Pref::AllowNotifs)) return;
    if (!trayHwnd)
        return;
    NOTIFYICONDATA notifyData = nid;
    notifyData.uFlags = NIF_INFO;
    notifyData.dwInfoFlags = NIIF_INFO;
    wcscpy_s(notifyData.szInfoTitle, title);
    wcscpy_s(notifyData.szInfo, message);
    Shell_NotifyIcon(NIM_MODIFY, &notifyData);
}