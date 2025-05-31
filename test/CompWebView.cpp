#include <Shlwapi.h>
#include <WebView2EnvironmentOptions.h>
#include <dcomp.h>

#include "WebView.h"

wil::com_ptr<ICoreWebView2Environment3> g_webviewEnvironment;
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

    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AdditionalBrowserArguments(L"--disable-gpu "
                                            L"--disable-software-rasterizer "
                                            L"--disable-gpu-compositing "
                                            L"--disable-gpu-vsync "
                                            L"--disable-accelerated-video-decode "
                                            L"--disable-accelerated-video-encode "
                                            L"--disable-background-networking "
                                            L"--disable-background-timer-throttling "
                                            L"--disable-breakpad "
                                            L"--disable-client-side-phishing-detection "
                                            L"--disable-default-apps "
                                            L"--disable-device-discovery-notifications "
                                            L"--disable-domain-reliability "
                                            L"--disable-features=AudioServiceOutOfProcess,IsolateOrigins,site-per-process,TranslateUI "
                                            L"--disable-hang-monitor "
                                            L"--disable-ipc-flooding-protection "
                                            L"--disable-popup-blocking "
                                            L"--disable-prompt-on-repost "
                                            L"--disable-renderer-backgrounding "
                                            L"--disable-sync "
                                            L"--disable-translate "
                                            L"--disable-web-resources "
                                            L"--disable-webrtc-hw-decoding "
                                            L"--disable-webrtc-hw-encoding "
                                            L"--disable-webrtc-multiple-routes "
                                            L"--disable-webrtc-stun-origin "
                                            L"--enable-low-end-device-mode "
                                            L"--enable-low-res-tiling "
                                            L"--enable-zero-copy "
                                            L"--metrics-recording-only "
                                            L"--no-first-run "
                                            L"--no-default-browser-check "
                                            L"--no-sandbox "
                                            L"--password-store=basic "
                                            L"--use-mock-keychain "
                                            L"--single-process "
                                            L"--no-zygote ");

    CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), options.Get(),
                                             Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                                                 [](HRESULT result, ICoreWebView2Environment *env) -> HRESULT
                                                 {
                                                     wprintf(L"[WebView] Created\n");
                                                     if (FAILED(result))
                                                     {
                                                         wprintf(L"[WebView] FAILED: HRESULT=0x%08X\n", result);
                                                         return result;
                                                     }
                                                     std::lock_guard<std::mutex> lock(g_envMutex);

                                                     wil::com_ptr<ICoreWebView2Environment3> env3;
                                                     HRESULT hrQ = env->QueryInterface(IID_PPV_ARGS(&env3));
                                                     if (FAILED(hrQ))
                                                     {
                                                         wprintf(L"[WebView] QI for ICoreWebView2Environment3 failed: 0x%08X\n", hrQ);
                                                         return hrQ;
                                                     }
                                                     g_webviewEnvironment = std::move(env3);

                                                     g_envReady = true;
                                                     g_envCv.notify_all();
                                                     wprintf(L"[WebView] WebView INITIALIZED\n");
                                                     return S_OK;
                                                 })
                                                 .Get());

    std::unique_lock<std::mutex> lock(g_envMutex);
    if (!g_envCv.wait_for(lock, std::chrono::seconds(10), []
                          { return g_envReady; }))
    {
        wprintf(L"[WebView] Timeout waiting for WebView2 environment\n");
        return;
    }
    wprintf(L"[WebView] Finished loading\n");
}

void InitializeWebView(HWND hwnd, const std::wstring &htmlRelativePath)
{
    std::shared_ptr<wil::com_ptr<ICoreWebView2CompositionController>> compController = std::make_shared<wil::com_ptr<ICoreWebView2CompositionController>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2>> webview = std::make_shared<wil::com_ptr<ICoreWebView2>>();

    g_webviewEnvironment->CreateCoreWebView2CompositionController(hwnd,
                                                       Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                                                           [hwnd, htmlRelativePath, compController, webview](HRESULT result, ICoreWebView2CompositionController *ctrl) -> HRESULT
                                                           {
                                                               if (FAILED(result))
                                                               {
                                                                   return result;
                                                               }

                                                               // retrieve base controller
                                                               *compController = ctrl;
                                                               Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller;
                                                               if (FAILED((*compController)->QueryInterface(IID_PPV_ARGS(&controller))))
                                                                   return E_FAIL;
                                                               controller->get_CoreWebView2(webview->put());

                                                               // initialize data struct
                                                               WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                                                               if (!data)
                                                                   return E_FAIL;
                                                               data->controller = controller;
                                                               data->compController = *compController;
                                                               data->webview = *webview;

                                                               // resize
                                                               RECT bounds;
                                                               GetClientRect(hwnd, &bounds);
                                                               controller->put_Bounds(bounds);

                                                               // handle native direct composition
                                                               Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice;
                                                               HRESULT hr = DCompositionCreateDevice2(nullptr, IID_PPV_ARGS(&dcompDevice));
                                                               if (FAILED(hr))
                                                                   return hr;
                                                               Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget;
                                                               hr = dcompDevice->CreateTargetForHwnd(hwnd, TRUE, &dcompTarget);
                                                               if (FAILED(hr))
                                                                   return hr;
                                                               Microsoft::WRL::ComPtr<IDCompositionVisual> rootVisual;
                                                               hr = (*compController)->get_RootVisualTarget(&rootVisual);
                                                               if (FAILED(hr))
                                                                   return hr;
                                                               hr = dcompTarget->SetRoot(rootVisual.Get());
                                                               if (FAILED(hr))
                                                                   return hr;
                                                               hr = dcompDevice->Commit();
                                                               if (FAILED(hr))
                                                                   return hr;

                                                               // disable unnecessary settings
                                                               wil::com_ptr<ICoreWebView2Settings> settings;
                                                               hr = (*webview)->get_Settings(&settings);
                                                               if (SUCCEEDED(hr) && settings)
                                                               {
                                                                   settings->put_IsStatusBarEnabled(FALSE);
                                                                   settings->put_AreDevToolsEnabled(FALSE);
                                                                   settings->put_IsZoomControlEnabled(FALSE);
                                                                   settings->put_AreDefaultContextMenusEnabled(FALSE);
                                                                   settings->put_AreHostObjectsAllowed(FALSE);
                                                               }

                                                               // set background to transparent
                                                               Microsoft::WRL::ComPtr<ICoreWebView2Controller2> controller2;
                                                               if (SUCCEEDED(controller->QueryInterface(IID_PPV_ARGS(&controller2))))
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
                                                               wprintf(L"[WebView] WebView WINDOW INITIALIZED\n");
                                                               wprintf(L"[WebView] Navigating to URL: %s\n", url.c_str());

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
                                                               (*webview)->add_NavigationCompleted(
                                                                   Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                                                       [](ICoreWebView2 *, ICoreWebView2NavigationCompletedEventArgs *args) -> HRESULT
                                                                       {
                                                                           BOOL success = FALSE;
                                                                           args->get_IsSuccess(&success);
                                                                           wprintf(L"[WebView] Navigation completed: success=%d\n", success);
                                                                           return S_OK;
                                                                       })
                                                                       .Get(),
                                                                   nullptr);

                                                               return S_OK;
                                                           })
                                                           .Get());
}

void ResizeWebView(HWND hwnd)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->controller)
    {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        data->controller->put_Bounds(bounds);
        if (data->compController)
        {
            Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice;
            if (SUCCEEDED(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&dcompDevice))))
            {
                wprintf(L"[WebView] REDRAWING\n");
                dcompDevice->Commit();
            }
        }
    }
}
