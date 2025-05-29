#pragma once

#include "../main.h"

RECT GetMonitorRect(HMONITOR hMon);
void AttachWindow(HWND hwnd);
HWND CreateWallpaperWindow(const std::wstring &htmlRelativePath);
void InitializeWallpaperWindows();
HWND CreateMainWindow();
WebViewData *GetWebViewData(HWND hwnd);