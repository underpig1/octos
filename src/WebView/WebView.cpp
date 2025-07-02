#include <Shlwapi.h>
#include <WebView2EnvironmentOptions.h>
#include <mutex>
#include <wil/com.h>
#include <wrl.h>
#include <dcomp.h>
#include <d3d11.h>

#include "WebView.h"
#include "../Bridge/Bridge.h"
#include "../Storage/Storage.h"
#include "../main.h"

wil::com_ptr<ICoreWebView2Environment3> g_webviewEnvironment;
std::mutex g_envMutex;
std::condition_variable g_envCv;
bool g_envReady = false;

Microsoft::WRL::ComPtr<ID3D11Device> g_d3dDevice;
Microsoft::WRL::ComPtr<IDXGIDevice> g_dxgiDevice;
Microsoft::WRL::ComPtr<IDCompositionDevice> g_dcompDevice;

void InitializeDCompDevice()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &g_d3dDevice,
        nullptr, nullptr);
    if (FAILED(hr))
        return;
    hr = g_d3dDevice.As(&g_dxgiDevice);
    if (FAILED(hr))
        return;
    hr = DCompositionCreateDevice(
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
                                            L"--no-sandbox "
                                            L"--disable-sync "
                                            L"--disable-background-networking "
                                            L"--disable-default-apps "
                                            L"--disable-translate "
                                            L"--disable-features=TranslateUI,AudioServiceOutOfProcess "
                                            L"--no-first-run "
                                            L"--no-default-browser-check "
                                            L"--disable-renderer-backgrounding "
                                            L"--disable-web-resources "
                                            L"--enable-low-end-device-mode "
                                            L"--password-store=basic "
                                            L"--use-mock-keychain ");

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

void OnWebViewControllerCreated(

    HWND hwnd,
    const std::wstring &htmlPath,
    wil::com_ptr<ICoreWebView2> webview,
    wil::com_ptr<ICoreWebView2Controller> controller)
{
    // resize
    RECT bounds;
    GetClientRect(hwnd, &bounds);
    controller->put_Bounds(bounds);
    controller->put_IsVisible(TRUE);

    // disable unnecessary settings
    wil::com_ptr<ICoreWebView2Settings> settings;
    HRESULT hr = webview->get_Settings(&settings);
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

    // Microsoft::WRL::ComPtr<ICoreWebView2Controller3> controller3;
    // if (SUCCEEDED(controller->QueryInterface(IID_PPV_ARGS(&controller3))))
    // {
    //     controller3->put_ShouldDetectMonitorScaleChanges(TRUE);
    //     controller3->put_RasterizationScale(10005);
    // }

    // navigate to local HTML file
    std::wstring url = htmlPath;
    if (hwnd == app_hwnd)
        url = ResolvePath(L"app/index.html", true);
    std::replace(url.begin(), url.end(), L'\\', L'/');
    webview->Navigate(url.c_str());
    wprintf(L"ATTACHING CONTROLLER WITH PATH %ws\n", url.c_str());
    // wprintf(L"EARLIER %ws\n", ResolvePath(L"app/index.html", true).c_str());
    // wprintf(L"SAME??? %b\n", url.c_str() == ResolvePath(L"app/index.html", true).c_str());

    // handle messages
    webview->add_WebMessageReceived(
        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [hwnd](ICoreWebView2 *, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT
            {
                wil::unique_cotaskmem_string messageRaw;
                if (SUCCEEDED(args->get_WebMessageAsJson(&messageRaw)))
                {
                    std::wstring msg = messageRaw.get();
                    if (hwnd == app_hwnd)
                        HandleWebMessage(msg, hwnd);
                }
                return S_OK;
            })
            .Get(),
        nullptr);
    wprintf(L"[WebViewAttach] Fully attached\n");
}

void AttachWebViewController(HWND hwnd, const std::wstring &htmlPath)
{
    // ShowWindow(hwnd, SW_SHOW);
    std::shared_ptr<wil::com_ptr<ICoreWebView2Controller>> controller = std::make_shared<wil::com_ptr<ICoreWebView2Controller>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2>> webview = std::make_shared<wil::com_ptr<ICoreWebView2>>();

    g_webviewEnvironment->CreateCoreWebView2Controller(hwnd,
                                                       Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                                           [hwnd, htmlPath, controller, webview](HRESULT result, ICoreWebView2Controller *ctrl) -> HRESULT
                                                           {
                                                               if (FAILED(result))
                                                                   return result;

                                                               *controller = ctrl;
                                                               (*controller)->get_CoreWebView2(webview->put());

                                                               (*webview)->OpenDevToolsWindow();

                                                               // initialize WebViewData
                                                               WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                                                               if (!data)
                                                                   return E_FAIL;
                                                               data->controller = *controller;
                                                               data->webview = *webview;

                                                               OnWebViewControllerCreated(hwnd, htmlPath, *webview, *controller);

                                                               return S_OK;
                                                           })
                                                           .Get());
}

void AttachWebViewCompositionController(HWND hwnd, const std::wstring &htmlPath)
{
    wprintf(L"[WebViewAttach] Started attaching...\n");
    std::shared_ptr<wil::com_ptr<ICoreWebView2Controller>> controller = std::make_shared<wil::com_ptr<ICoreWebView2Controller>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2CompositionController>> compController = std::make_shared<wil::com_ptr<ICoreWebView2CompositionController>>();
    std::shared_ptr<wil::com_ptr<ICoreWebView2>> webview = std::make_shared<wil::com_ptr<ICoreWebView2>>();

    HRESULT hr = g_webviewEnvironment->CreateCoreWebView2CompositionController(hwnd,
                                                                               Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                                                                                   [hwnd, htmlPath, compController, controller, webview](HRESULT result, ICoreWebView2CompositionController *ctrl) -> HRESULT
                                                                                   {
                                                                                       if (FAILED(result))
                                                                                           return result;

                                                                                       // query base controller
                                                                                       *compController = ctrl;
                                                                                       compController->query_to(controller->put());

                                                                                       // get webview
                                                                                       (*controller)->get_CoreWebView2(webview->put());
                                                                                       //    (*controller)->put_IsVisible(TRUE);

                                                                                       // attach DC pipeline
                                                                                       Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget;
                                                                                       Microsoft::WRL::ComPtr<IDCompositionVisual> dcompVisual;
                                                                                       g_dcompDevice->CreateTargetForHwnd(hwnd, FALSE, &dcompTarget);
                                                                                       g_dcompDevice->CreateVisual(&dcompVisual);
                                                                                       dcompTarget->SetRoot(dcompVisual.Get());
                                                                                       ctrl->put_RootVisualTarget(dcompVisual.Get());

                                                                                       // initialize WebViewData
                                                                                       WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                                                                                       if (!data)
                                                                                           return E_FAIL;
                                                                                       data->compController = *compController;
                                                                                       data->controller = *controller;
                                                                                       data->webview = *webview;
                                                                                       data->dcompTarget = dcompTarget;
                                                                                       data->dcompVisual = dcompVisual;
                                                                                       g_dcompDevice->Commit();

                                                                                       OnWebViewControllerCreated(hwnd, htmlPath, *webview, *controller);

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
        if (hwnd != app_hwnd)
            g_dcompDevice->Commit();
    }
}

void NavigateWindow(HWND hwnd, std::wstring url)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->webview)
    {
        std::replace(url.begin(), url.end(), L'\\', L'/');
        wprintf(L"TRYING TO NAVIGATE TO %ws", url.c_str());
        // RECT bounds;
        // GetWindowRect(hwnd, &bounds);
        // data->controller->put_IsVisible(TRUE);
        // data->controller->put_Bounds(bounds);
        data->webview->Navigate(url.c_str());
        g_dcompDevice->Commit();
    }
}

void HandleOnDestroy(HWND hwnd)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data)
    {
        if (data->controller)
        {
            data->controller->Close();
            data->controller.reset();
            data->webview.reset();
        }
        delete data;
    }
    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
}

void HandleDPIChange(HWND hwnd, LPARAM lParam)
{
    if (!IsWindow(hwnd) || !IsWindowVisible(hwnd))
        return;
    RECT *const prcNewWindow = (RECT *)lParam;
    SetWindowPos(hwnd, NULL,
                 prcNewWindow->left,
                 prcNewWindow->top,
                 prcNewWindow->right - prcNewWindow->left,
                 prcNewWindow->bottom - prcNewWindow->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->controller)
    {
        int width = prcNewWindow->right - prcNewWindow->left;
        int height = prcNewWindow->bottom - prcNewWindow->top;
        RECT bounds = {0, 0, width, height};
        data->controller->put_Bounds(bounds);
        if (hwnd != app_hwnd)
            g_dcompDevice->Commit();
    }
}

void SetWebViewVisibility(HWND hwnd, bool visible)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->controller)
    {
        data->registerInput = visible;
        bool memorySaver = GetPref(Pref::MemorySaver);
        if (memorySaver)
        {
            if (data->hidden == visible)
            {
                data->controller->put_IsVisible(visible);
                data->hidden = !visible;
            }
        }
        else if (data->hidden)
        {
            data->controller->put_IsVisible(true);
            data->hidden = false;
        }
    }
}