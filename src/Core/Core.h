#pragma once

#include <windows.h>
#include <vector>
#include <string>

struct MonitorWindow
{
    HWND hwnd;
    HMONITOR monitor;
    bool fixing = false;
    std::wstring htmlPath = L"assets/index.html";
    void ExpandToMonitor();
};

extern std::vector<MonitorWindow> ms;
extern std::vector<HMONITOR> g_monitors;

RECT GetMonitorRect(HMONITOR hMon);
void AttachWindow(HWND hwnd);
HWND CreateWallpaperWindow(const std::wstring &htmlRelativePath);
void InitializeWallpaperWindows();
HWND CreateMainWindow();
void RecreateWindow(LPARAM lParam);
void RecreateWallpapers();
void CreateMonitorWindow(HMONITOR hMon);
void SetWallpaperVisibility(bool visible = true);
std::vector<std::wstring> GetMonitorIds();
void NavigateWallpaperByMonitorId(std::wstring monitorId, std::wstring url);
void ReleaseMainWindow();
void ReattachMainWindow();
void NavigateAllWallpapers(std::wstring url);
MonitorWindow *FindMonitorWindowById(const std::wstring monitorId);
std::vector<std::wstring> FindMonitorIdsByHwnd(HWND hwnd);