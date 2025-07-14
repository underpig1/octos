#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <bitset>
#include <mutex>

#define WM_RECREATEHWND (WM_USER + 1)
#define WM_DESTROYTRIGGER (WM_USER + 2)
#define WM_TRAYICON (WM_USER + 3)
#define WM_CLOSEAPP (WM_USER + 4)
#define WM_DISPATCHJSON (WM_USER + 5)
#define WM_RESTOREMAINWINDOW (WM_USER + 6)
#define WM_OPENDEVTOOLS (WM_USER + 7)

#define CLASS_NAME L"OctosWorker"
#define GLOBAL_MUTEX_NAME L"Global\\OctosMutex"

extern HINSTANCE g_hInstance;
extern HWND app_hwnd;
extern bool g_appHwndAttached;
extern HICON g_hIcon;

extern std::mutex app_hwnd_mutex;
extern std::condition_variable app_hwnd_cv;
extern std::mutex all_hwnd_mutex;
extern std::condition_variable all_hwnd_cv;

enum Pref
{
    MemorySaver,
    DisableMouseInput,
    RunOnStartup,
    EnableGPU,
    Count
};

extern std::bitset<static_cast<size_t>(Pref::Count)> g_prefs;

inline bool GetPref(Pref key)
{
    return g_prefs.test(static_cast<size_t>(key));
}

inline void SetPref(Pref key, bool value)
{
    g_prefs.set(static_cast<size_t>(key), value);
}

void RestartApp();