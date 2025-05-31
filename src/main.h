#pragma once

#include <windows.h>
#include <vector>
#include <string>

#define WM_RECREATEHWND (WM_USER + 1)
#define WM_DESTROYTRIGGER (WM_USER + 2)
#define WM_TRAYICON (WM_USER + 3)

extern const std::wstring defaultHtmlPath;
extern const wchar_t CLASS_NAME[];
extern HINSTANCE g_hInstance;
extern HWND app_hwnd;