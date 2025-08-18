#include "Core/Core.h"
#include "TrayIcon/TrayIcon.h"
#include "Watchdog/Watchdog.h"
#include "WebView/WebView.h"
#include "Event/Event.h"
#include "main.h"
#include "Storage/Storage.h"
#include "Bridge/Bridge.h"
#include "API/Media.h"
#include "CLI/CLI.h"
#include "API/Audio.h"
#include "WebView/Bootstrap.h"

HINSTANCE g_hInstance;
HWND app_hwnd;
std::mutex app_hwnd_mutex;
std::condition_variable app_hwnd_cv;
std::mutex all_hwnd_mutex;
std::condition_variable all_hwnd_cv;
bool g_appHwndAttached = false;
HICON g_hIcon;
HANDLE g_hMutex;

std::bitset<static_cast<size_t>(Pref::Count)> g_prefs = []
{
    std::bitset<static_cast<size_t>(Pref::Count)> defaults;
    defaults.set(static_cast<size_t>(Pref::MemorySaver), true);
    defaults.set(static_cast<size_t>(Pref::DisableMouseInput), false);
    defaults.set(static_cast<size_t>(Pref::RunOnStartup), true);
    defaults.set(static_cast<size_t>(Pref::EnableGPU), true);
    defaults.set(static_cast<size_t>(Pref::EnableSandboxing), false);
    defaults.set(static_cast<size_t>(Pref::AllowNotifs), true);
    return defaults;
}();

void OnClose()
{
    KillTimer(app_hwnd, 1);
    DestroyTrayIcon();
    CoUninitialize();
    UninstallEventHooks();
    ReleaseMutex(g_hMutex);
    CloseHandle(g_hMutex);
    PostQuitMessage(0);
}

void RestartApp()
{
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    if (CreateProcess(szPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        OnClose();
        ExitProcess(0);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // if (msg == WM_DESTROY || msg == WM_CLOSE)
    // {
    //     wprintf(L"we have an issue");
    // }
    switch (msg)
    {
    case WM_NCCALCSIZE:
    {
        if (hwnd == app_hwnd && wParam == TRUE)
        {
            wprintf(L"WE ARE RUNNING\n");
            NCCALCSIZE_PARAMS *pParams = (NCCALCSIZE_PARAMS *)lParam;
            const int border = 8;
            pParams->rgrc[0].left += border;
            pParams->rgrc[0].right -= border;
            pParams->rgrc[0].bottom -= border;
            return 0;
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        if (hwnd == app_hwnd)
        {
            auto mmi = reinterpret_cast<MINMAXINFO *>(lParam);
            mmi->ptMinTrackSize.x = 850;
            mmi->ptMinTrackSize.y = 500;
            return 0;
        }
        break;
    }
    case WM_DPICHANGED:
        HandleDPIChange(hwnd, lParam);
        return 0;
    // case WM_NCHITTEST:
    // {
    //     POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    //     ScreenToClient(hwnd, &pt);
    //     RECT rc;
    //     GetClientRect(hwnd, &rc);
    //     const int BORDER_WIDTH = 8;

    //     if (pt.x < BORDER_WIDTH)
    //     {
    //         if (pt.y < BORDER_WIDTH)
    //             return HTTOPLEFT;
    //         else if (pt.y > rc.bottom - BORDER_WIDTH)
    //             return HTBOTTOMLEFT;
    //         else
    //             return HTLEFT;
    //     }
    //     else if (pt.x > rc.right - BORDER_WIDTH)
    //     {
    //         if (pt.y < BORDER_WIDTH)
    //             return HTTOPRIGHT;
    //         else if (pt.y > rc.bottom - BORDER_WIDTH)
    //             return HTBOTTOMRIGHT;
    //         else
    //             return HTRIGHT;
    //     }
    //     else if (pt.y < BORDER_WIDTH)
    //         return HTTOP;
    //     else if (pt.y > rc.bottom - BORDER_WIDTH)
    //         return HTBOTTOM;
    //     return HTCLIENT;
    // }
    case WM_NCCREATE:
        wprintf(L"[WinMain] Created\n");
        HandleOnCreate(hwnd, lParam);
        return TRUE;
    case WM_SIZE:
        wprintf(L"[WinMain] Resized\n");
        if (wParam == SIZE_RESTORED)
            OnMainWindowRestore();
        HandleResize(hwnd, lParam);
        return 0;
    case WM_ACTIVATE:
        for (auto &mw : ms)
            FixWallpaperOrder(mw.hwnd);
        break;
    case WM_SETFOCUS:
        for (auto &mw : ms)
            FixWallpaperOrder(mw.hwnd);
        break;
    case WM_KILLFOCUS:
        for (auto &mw : ms)
            FixWallpaperOrder(mw.hwnd);
        break;
    case WM_SHOWWINDOW:
    {
        wprintf(L"[WinMain] Shown %d\n", g_appHwndAttached);
        if (wParam && app_hwnd == hwnd && !g_appHwndAttached)
        {
            wprintf(L"[WinMain] Attaching webview controller.... CHECK IF RUNS MULTIPLE TIMES\n");
            AttachWebViewController(hwnd, L"app/index.html");
            g_appHwndAttached = true;
        }
        return 0;
    }
    case WM_DISPLAYCHANGE:
        DispatchMonitorData();
        break;
    case WM_CLOSE:
    {
        wprintf(L"[WinMain] Closed\n");
        if (app_hwnd == hwnd)
            ReleaseMainWindow();
        return 0;
    }
    case WM_DESTROY:
    {
        wprintf(L"[WinMain] Destroyed\n");
        SubscriptionCleanup(hwnd);
        RemoveAudioSubscription(hwnd);
        HandleOnDestroy(hwnd);
        return 0;
    }
    case WM_TIMER:
        WatchdogProc();
        return 0;
    case WM_RECREATEHWND:
    {
        wprintf(L"[WinMain] Recreated\n");
        RecreateWindow(lParam);
        return 0;
    }
    case WM_DESTROYTRIGGER:
    {
        wprintf(L"[WinMain] Destroy triggered\n");
        HWND re_hwnd = reinterpret_cast<HWND>(lParam);
        if (IsWindow(re_hwnd))
            DestroyWindow(re_hwnd);
        return 0;
    }
    case WM_TRAYICON:
        wprintf(L"[WinMain] Tray created\n");
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
            ShowTrayMenu();
        return 0;
    case WM_CLOSEAPP:
        OnClose();
        return 0;
    case WM_DISPATCHJSON:
    {
        std::wstring *message = reinterpret_cast<std::wstring *>(lParam);
        // wprintf(L"\n\nWERE DISPATCHING FROM MAIN THREAD %ws\n\n", (*message).c_str());
        DispatchToHwnd(hwnd, *message);
        delete message;
        return 0;
    }
    case WM_COPYDATA:
    {
        // wprintf(L"/ ///  / // I GOT SOMEONE ELSES DATA");
        PCOPYDATASTRUCT pCDS = (PCOPYDATASTRUCT)lParam;
        LPWSTR arg = (LPWSTR)(pCDS->lpData);
        ParseCommandLineArgs(arg);
        return 0;
    }
    case WM_OPENDEVTOOLS:
    {
        OpenDevTools(hwnd);
        return 0;
    }
    case WM_RESTOREMAINWINDOW:
    {
        RestoreMainWindow();
        return 0;
    }
    default:
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void RegisterWndClass(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    g_hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    HBRUSH bgBrush = CreateSolidBrush(RGB(26, 26, 26));
    wc.hbrBackground = bgBrush;
    std::wstring iconPath = ResolvePath(L"img\\icon.ico");
    g_hIcon = (HICON)LoadImageW(nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (!g_hIcon)
        g_hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIcon = g_hIcon;
    RegisterClass(&wc);
}

bool HandleInstances(LPWSTR lpCmdLine)
{
    g_hMutex = CreateMutexW(nullptr, TRUE, GLOBAL_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND hwndOther = FindWindowW(CLASS_NAME, nullptr);
        if (hwndOther)
        {
            if (lpCmdLine && *lpCmdLine != L'\0')
            {
                auto cmdLine = GetCommandLineW();
                COPYDATASTRUCT cds;
                cds.dwData = 1;
                cds.cbData = (wcslen(cmdLine) + 1) * sizeof(wchar_t);
                cds.lpData = cmdLine;
                SendMessageW(hwndOther, WM_COPYDATA, 0, (LPARAM)&cds);
            }
            else
            {
                wprintf(L"\n\n################### HELLO\n\n");
                SendMessageW(hwndOther, WM_RESTOREMAINWINDOW, 0, 0);
            }
        }
        return true;
    }
    ParseCommandLineArgs(GetCommandLineW());
    wprintf(L"\n\n###### handling base case\n\n");
    return false;
}

bool AddToStartup()
{
    const std::wstring &exePath = GetAppPath();
    HKEY hKey;
    const wchar_t *runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return false;
    LONG result = RegSetValueExW(
        hKey,
        CLASS_NAME,
        0,
        REG_SZ,
        reinterpret_cast<const BYTE *>(exePath.c_str()),
        static_cast<DWORD>((exePath.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RemoveFromStartup()
{
    HKEY hKey;
    const wchar_t *runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return false;
    LONG result = RegDeleteValueW(hKey, CLASS_NAME);
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}

bool HasSetStartupOnce()
{
    HKEY hKey;
    DWORD value = 0;
    DWORD size = sizeof(DWORD);

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Octos", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    if (RegQueryValueExW(hKey, L"StartupSet", nullptr, nullptr, (LPBYTE)&value, &size) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return value == 1;
}

void MarkStartupSet()
{
    HKEY hKey;
    DWORD value = 1;

    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Octos", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, L"StartupSet", 0, REG_DWORD, (const BYTE *)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void HandleStartup()
{
    if (!HasSetStartupOnce())
    {
        AddToStartup();
        MarkStartupSet();
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    if (HandleInstances(lpCmdLine))
        return 0;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // AllocConsole();
    // freopen("CONOUT$", "w", stdout);
    // freopen("CONOUT$", "w", stderr);
    // freopen("CONIN$", "r", stdin);

    DWORD g_mainThreadId = GetCurrentThreadId();
    RegisterWndClass(hInstance);
    HandleBootstrap([g_mainThreadId]()
                    { PostThreadMessage(g_mainThreadId, WM_USER + 10, 0, 0); });

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (msg.message == WM_USER + 10)
        {
            LoadAndHandleAppPrefs();
            HandleStartup();
            InitializeWebViewEnvironment();
            InitializeTrayIcon();
            CreateMainWindow();
            InitializeWallpaperWindows();
            InstallEventHooks();
            wprintf(L"[WinMain] Initialized\n");
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    OnClose();
    return (int)msg.wParam;
}