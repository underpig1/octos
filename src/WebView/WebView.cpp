#include <Shlwapi.h>

#include "WebView.h"

wil::com_ptr<ICoreWebView2Environment> g_webviewEnvironment;
std::mutex g_envMutex;
std::condition_variable g_envCv;
bool g_envReady = false;

void InitializeWebViewEnvironment()
{
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring userDataFolder = tempPath;
    userDataFolder += L"WebView2UserData";

    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrInit))
        return;
    CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), nullptr,
                                             Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                                                 [](HRESULT result, ICoreWebView2Environment *env) -> HRESULT
                                                 {
                                                     wprintf(L"[WebViewEnv] Created\n");
                                                     if (FAILED(result))
                                                     {
                                                         wprintf(L"[WebViewEnv] FAILED: HRESULT=0x%08X\n", result);
                                                         return result;
                                                     }
                                                     std::lock_guard<std::mutex> lock(g_envMutex);
                                                     g_webviewEnvironment = env;
                                                     g_envReady = true;
                                                     g_envCv.notify_all();
                                                     return S_OK;
                                                 })
                                                 .Get());
    std::unique_lock<std::mutex> lock(g_envMutex);
    if (!g_envCv.wait_for(lock, std::chrono::seconds(10), []
                          { return g_envReady; }))
    {
        wprintf(L"[AttachWebView] Timeout waiting for WebView2 environment\n");
        return;
    }
}

void InitializeWebView(HWND hwnd, const std::wstring &htmlRelativePath)
{
    std::shared_ptr<wil::com_ptr<ICoreWebView2Controller>> controller = std::make_shared<wil::com_ptr<ICoreWebView2Controller>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2>> webview = std::make_shared<wil::com_ptr<ICoreWebView2>>();

    g_webviewEnvironment->CreateCoreWebView2Controller(hwnd,
                                                       Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                                           [hwnd, htmlRelativePath, controller, webview](HRESULT result, ICoreWebView2Controller *ctrl) -> HRESULT
                                                           {
                                                               if (FAILED(result))
                                                               {
                                                                   return result;
                                                               }

                                                               *controller = ctrl;
                                                               (*controller)->get_CoreWebView2(webview->put());

                                                               // Store in window user data (assuming WebViewData allocated earlier)
                                                               WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                                                               if (!data)
                                                               {
                                                                   return E_FAIL;
                                                               }
                                                               data->controller = *controller;
                                                               data->webview = *webview;

                                                               // resize
                                                               RECT bounds;
                                                               GetClientRect(hwnd, &bounds);
                                                               (*controller)->put_Bounds(bounds);

                                                               // set background to transparent
                                                               Microsoft::WRL::ComPtr<ICoreWebView2Controller2> controller2;
                                                               if (SUCCEEDED((*controller)->QueryInterface(IID_PPV_ARGS(&controller2))))
                                                               {
                                                                   controller2->put_DefaultBackgroundColor({0, 0, 0, 0});
                                                               }

                                                               // navigate to local HTML file
                                                               wchar_t exePath[MAX_PATH];
                                                               GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                                                               PathRemoveFileSpecW(exePath);
                                                               std::wstring htmlPath = std::wstring(exePath) + L"\\" + htmlRelativePath;
                                                               for (auto &c : htmlPath)
                                                                   if (c == L'\\')
                                                                       c = L'/';
                                                               std::wstring url = L"file:///" + htmlPath;
                                                               (*webview)->Navigate(url.c_str());

                                                               // handle messages
                                                               (*webview)->add_WebMessageReceived(
                                                                   Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                                                       [hwnd](ICoreWebView2 *, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT
                                                                       {
                                                                           wil::unique_cotaskmem_string messageRaw;
                                                                           if (SUCCEEDED(args->get_WebMessageAsJson(&messageRaw)))
                                                                           {
                                                                               std::wstring msg = messageRaw.get();
                                                                               if (msg.find(L"\"type\":\"drag\"") != std::wstring::npos)
                                                                               {
                                                                                   ReleaseCapture();
                                                                                   SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                                                                               }
                                                                           }
                                                                           return S_OK;
                                                                       })
                                                                       .Get(),
                                                                   nullptr);
                                                               return S_OK;
                                                           })
                                                           .Get());
}