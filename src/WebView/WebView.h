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
};

void InitializeWebViewEnvironment();
void AttachWebView(HWND hwnd, const std::wstring &htmlRelativePath);
void ResizeWebView(HWND hwnd);
void HandleOnCreate(HWND hwnd, LPARAM lParam);
void HandleResize(HWND hwnd, LPARAM lParam);
void HandleOnDestroy(HWND hwnd);