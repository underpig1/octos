#include <Shlwapi.h>
#include <WebView2EnvironmentOptions.h>
#include <mutex>
#include <wil/com.h>
#include <wrl.h>
#include <dcomp.h>
#include <d3d11.h>

#include "WebView.h"

wil::com_ptr<ICoreWebView2Environment3> g_webviewEnvironment;
std::mutex g_envMutex;
std::condition_variable g_envCv;
bool g_envReady = false;

Microsoft::WRL::ComPtr<ID3D11Device> g_d3dDevice;
Microsoft::WRL::ComPtr<IDXGIDevice> g_dxgiDevice;
Microsoft::WRL::ComPtr<IDCompositionDevice> g_dcompDevice;

void InitializeDCompDevice()
{
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &g_d3dDevice,
        nullptr, nullptr);
    g_d3dDevice.As(&g_dxgiDevice);
    DCompositionCreateDevice(
        g_dxgiDevice.Get(),
        __uuidof(IDCompositionDevice),
        &g_dcompDevice);
}

void InitializeWebViewEnvironment()
{
    wprintf(L"[WebViewEnv] Started creating environment...\n");
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring userDataFolder = tempPath;
    userDataFolder += L"WebView2UserData";

    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrInit))
        return;

    InitializeDCompDevice();

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
                                                     if (FAILED(result))
                                                         return result;

                                                     Microsoft::WRL::ComPtr<ICoreWebView2Environment3> env3;
                                                     env->QueryInterface(IID_PPV_ARGS(&env3));
                                                     if (!env3)
                                                         return E_FAIL;

                                                     std::lock_guard<std::mutex> lock(g_envMutex);
                                                     g_webviewEnvironment = env3;
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
    wprintf(L"[WebViewEnv] Created WebView env\n");
}

void AttachWebView(HWND hwnd, const std::wstring &htmlRelativePath)
{
    wprintf(L"[WebViewAttach] Started attaching...\n");
    std::shared_ptr<wil::com_ptr<ICoreWebView2Controller>> controller = std::make_shared<wil::com_ptr<ICoreWebView2Controller>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2CompositionController>> compController = std::make_shared<wil::com_ptr<ICoreWebView2CompositionController>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2>> webview = std::make_shared<wil::com_ptr<ICoreWebView2>>();

    // HWND hwnd = CreateWindowEx(
    //     WS_EX_NOACTIVATE,
    //     L"MyClass",
    //     L"MyWindow",
    //     WS_POPUP, // not WS_CHILD, not 0
    //     0, 0, 100, 100,
    //     NULL, NULL, GetModuleHandle(nullptr), NULL);
    // if (GetAncestor(hwnd, GA_ROOT) == HWND_MESSAGE)
    // {
    //     wprintf(L"[WebViewAttach] FUCK MY LIFE FUCK FUCK FUCK EVERYTHING\n");
    // }

    HRESULT hr = g_webviewEnvironment->CreateCoreWebView2CompositionController(hwnd,
                                                                  Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                                                                      [hwnd, htmlRelativePath, compController, controller, webview](HRESULT result, ICoreWebView2CompositionController *ctrl) -> HRESULT
                                                                      {
                                                                          if (FAILED(result))
                                                                              return result;

                                                                          // query base controller
                                                                          *compController = ctrl;
                                                                          Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller;
                                                                          ctrl->QueryInterface(IID_PPV_ARGS(&controller));

                                                                          // get webview
                                                                          controller->get_CoreWebView2(webview->put());
                                                                          controller->put_IsVisible(TRUE);

                                                                          // attach DC pipeline
                                                                          Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget;
                                                                          Microsoft::WRL::ComPtr<IDCompositionVisual> dcompVisual;
                                                                          g_dcompDevice->CreateTargetForHwnd(hwnd, TRUE, &dcompTarget);
                                                                          g_dcompDevice->CreateVisual(&dcompVisual);
                                                                          dcompTarget->SetRoot(dcompVisual.Get());
                                                                          ctrl->put_RootVisualTarget(dcompVisual.Get());

                                                                          // initialize WebViewData
                                                                          WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                                                                          if (!data)
                                                                              return E_FAIL;
                                                                          data->compController = *compController;
                                                                          data->controller = controller;
                                                                          data->webview = *webview;
                                                                          data->dcompTarget = dcompTarget;
                                                                          data->dcompVisual = dcompVisual;

                                                                          // resize
                                                                          RECT bounds;
                                                                          GetClientRect(hwnd, &bounds);
                                                                          controller->put_Bounds(bounds);
                                                                          g_dcompDevice->Commit();

                                                                          // disable unnecessary settings
                                                                          wil::com_ptr<ICoreWebView2Settings> settings;
                                                                          HRESULT hr = (*webview)->get_Settings(&settings);
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
                                                                          wprintf(L"[WebViewAttach] Fully attached\n");
                                                                          return S_OK;
                                                                      })
                                                                      .Get());
    wprintf(L"[WebViewAttach] CreateCoreWebView2CompositionController returned: 0x%08X\n", hr);
}

void HandleOnCreate(HWND hwnd, LPARAM lParam)
{
    CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
    WebViewData *data = reinterpret_cast<WebViewData *>(cs->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    wprintf(L"[WebView] Created\n");
}

void HandleResize(HWND hwnd, LPARAM lParam)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->controller)
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        RECT bounds = {0, 0, width, height};
        data->controller->put_Bounds(bounds);
        g_dcompDevice->Commit();
    }
}

void HandleOnDestroy(HWND hwnd)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    data->controller->Close();
    data->controller.reset();
    data->webview.reset();
    delete data;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
}