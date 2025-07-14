#pragma once

#include <wil/com.h>
#include <wrl.h>
#include <WebView2.h>
#include <dcomp.h>

struct WebViewData
{
    wil::com_ptr<ICoreWebView2Controller> controller;
    wil::com_ptr<ICoreWebView2> webview;
    wil::com_ptr<ICoreWebView2CompositionController> compController;
    Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget;
    Microsoft::WRL::ComPtr<IDCompositionVisual> dcompVisual;
    bool hidden = false;
    bool registerInput = true;
};

void InitializeWebViewEnvironment();
void AttachWebViewController(HWND hwnd, const std::wstring &htmlPath);
void AttachWebViewCompositionController(HWND hwnd, const std::wstring &htmlPath);
void HandleOnCreate(HWND hwnd, LPARAM lParam);
void HandleResize(HWND hwnd, LPARAM lParam);
void HandleOnDestroy(HWND hwnd);
void HandleDPIChange(HWND hwnd, LPARAM lParam);
void NavigateWindow(HWND hwnd, std::wstring url);
void SetWebViewVisibility(HWND hwnd, bool visible);
void ReloadWebViewWindow(HWND hwnd);
void OpenDevTools(HWND hwnd);