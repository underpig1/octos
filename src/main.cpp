#include "main.h"
#include <vector>
#include <memory>
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define TRAY_ICON_UID 1001

NOTIFYICONDATA nid = {};
HICON hIcon = nullptr;

std::unique_ptr<WebViewApp> app;
std::vector<std::unique_ptr<WebViewApp>> wps;

void ShowTrayMenu(HWND hwnd);
void RemoveTrayIcon();

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
    if (cmd == 1 && app)
    {
        app->Show();
    }
    else if (cmd == 2 && app)
    {
        app->Hide();
    }
    else if (cmd == 3)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
        {
            ShowTrayMenu(hwnd);
        }
        break;
    case WM_DESTROY:
        RemoveTrayIcon();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    app = std::make_unique<WebViewApp>();
    app->Init(hInstance, L"assets/index.html", nullptr, L"assets/img/icon.ico");
    HWND hwnd1 = app->GetHwnd();

    auto wallpaper = std::make_unique<WebViewApp>();
    wallpaper->Init(hInstance, L"assets/second.html");
    wps.push_back(std::move(wallpaper));

    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayWindow";
    RegisterClass(&wc);
    HWND trayHwnd = CreateWindowEx(0, L"TrayWindow", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
    AddTrayIcon(trayHwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}