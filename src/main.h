#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <bitset>

#define WM_RECREATEHWND (WM_USER + 1)
#define WM_DESTROYTRIGGER (WM_USER + 2)
#define WM_TRAYICON (WM_USER + 3)
#define WM_CLOSEAPP (WM_USER + 4)
#define WM_DISPATCHJSON (WM_USER + 5)

extern const wchar_t CLASS_NAME[];
extern HINSTANCE g_hInstance;
extern HWND app_hwnd;
extern bool g_appHwndAttached;
extern HICON g_hIcon;

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