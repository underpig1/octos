#include "main.h"
#include <wrl.h>
#include <shellscalingapi.h>
#include <shlwapi.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

using namespace Microsoft::WRL;

WebViewApp::WebViewApp() {}
WebViewApp::~WebViewApp() {}

void WebViewApp::Init(HINSTANCE hInstance, const std::wstring &htmlRelativePath, HWND parentHwnd, const std::wstring &iconPath)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    const wchar_t CLASS_NAME[] = L"WebView2Window";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WebViewApp::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    if (!iconPath.empty())
    {
        wc.hIcon = (HICON)LoadImageW(nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    }
    RegisterClass(&wc);

    DWORD style = parentHwnd ? (WS_CHILD | WS_VISIBLE) : (WS_POPUP | WS_VISIBLE);
    DWORD exStyle = parentHwnd ? 0 : (iconPath.empty() ? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW);

    m_hwnd = CreateWindowExW(exStyle, CLASS_NAME, L"WebView2 Window", style, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, parentHwnd, nullptr, hInstance, this);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring userDataFolder = tempPath;
    userDataFolder += L"WebView2Data";

    CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this, htmlRelativePath](HRESULT result, ICoreWebView2Environment *env) -> HRESULT
            {
                if (FAILED(result))
                    return result;
                env->CreateCoreWebView2Controller(m_hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this, htmlRelativePath](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT
                        {
                            if (FAILED(result))
                                return result;
                            m_controller = controller;
                            m_controller->get_CoreWebView2(&m_webview);
                            RECT bounds;
                            GetClientRect(m_hwnd, &bounds);
                            m_controller->put_Bounds(bounds);

                            // Set WebView2 background to transparent
                            if (m_controller) {
                                ICoreWebView2Controller2* controller2 = nullptr;
                                if (SUCCEEDED(m_controller->QueryInterface(IID_PPV_ARGS(&controller2))) && controller2) {
                                    controller2->put_DefaultBackgroundColor({ 0, 0, 0, 0 }); // RGBA, alpha=0
                                    controller2->Release();
                                }
                            }

                            wchar_t exePath[MAX_PATH];
                            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                            PathRemoveFileSpecW(exePath);
                            std::wstring htmlPath = std::wstring(exePath) + L"\\" + htmlRelativePath;
                            for (auto &c : htmlPath)
                                if (c == L'\\')
                                    c = L'/';
                            std::wstring url = L"file:///" + htmlPath;
                            m_webview->Navigate(url.c_str());

                            if (m_webview) {
                                m_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        wil::unique_cotaskmem_string messageRaw;
                                        if (SUCCEEDED(args->get_WebMessageAsJson(&messageRaw))) {
                                            std::wstring msg = messageRaw.get();
                                            if (msg.find(L"\"type\":\"drag\"") != std::wstring::npos) {
                                                ReleaseCapture();
                                                SendMessage(m_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                                            }
                                        }
                                        return S_OK;
                                    }).Get(), nullptr);
                            }

                            return S_OK;
                        })
                        .Get());
                return S_OK;
            })
            .Get());
}

LRESULT CALLBACK WebViewApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WebViewApp *app = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
        app = reinterpret_cast<WebViewApp *>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    else
    {
        app = reinterpret_cast<WebViewApp *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (uMsg)
    {
    case WM_SIZE:
        if (app && app->m_controller)
        {
            RECT bounds;
            GetClientRect(hwnd, &bounds);
            app->m_controller->put_Bounds(bounds);
        }
        break;
    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0); // Now actually quit the app when window is destroyed
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void WebViewApp::Show()
{
    if (m_hwnd)
    {
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
    }
}

void WebViewApp::Hide()
{
    if (m_hwnd)
    {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

HWND WebViewApp::GetHwnd() const
{
    return m_hwnd;
}
