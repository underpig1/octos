#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include <wil/com.h>
#include <WebView2.h>

class WebViewApp
{
public:
    WebViewApp();
    ~WebViewApp();

    void Init(HINSTANCE hInstance, const std::wstring &htmlRelativePath, HWND parentHwnd = nullptr, const std::wstring &iconPath = L"");
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Show();
    void Hide();
    HWND GetHwnd() const;

    HWND m_hwnd = nullptr;
    wil::com_ptr<ICoreWebView2Controller> m_controller;
    wil::com_ptr<ICoreWebView2> m_webview;
};