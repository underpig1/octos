#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <wrl.h>
#include <WebView2.h>
#include <wil/com.h>

#define WM_RECREATEHWND (WM_USER + 1)
#define WM_DESTROYTRIGGER (WM_USER + 2)
#define WM_TRAYICON (WM_USER + 3)

struct MonitorWindow
{
    HWND hwnd;
    HMONITOR monitor;
    bool fixing = false;
    std::wstring htmlPath = L"assets/index.html";
    void ExpandToMonitor();
};

struct WebViewData
{
    wil::com_ptr<ICoreWebView2Controller> controller;
    wil::com_ptr<ICoreWebView2> webview;
};

extern const std::wstring defaultHtmlPath;
extern const wchar_t CLASS_NAME[];
extern HINSTANCE g_hInstance;
extern HWND app_hwnd;
extern std::vector<MonitorWindow> ms;
extern std::vector<HMONITOR> g_monitors;