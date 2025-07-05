// File: main.cpp
// Compile with: cl /EHsc /std:c++17 main.cpp d3d11.lib dcomp.lib windowscodecs.lib ole32.lib oleaut32.lib runtimeobject.lib WebView2LoaderStatic.lib

#include <windows.h>
#include <d3d11.h>
#include <dcomp.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include <windowsx.h>

using namespace Microsoft::WRL;

HWND g_hostHwnd = nullptr;
ComPtr<ID3D11Device> g_d3dDevice;
ComPtr<IDXGIDevice> g_dxgiDevice;
ComPtr<IDCompositionDevice> g_dcompDevice;
ComPtr<IDCompositionTarget> g_dcompTarget;
ComPtr<IDCompositionVisual> g_dcompVisual;
ComPtr<ICoreWebView2CompositionController> g_compController;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void InitDCompPipeline();
void CreateWebView2Composition();
void ForwardMouse(UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    HWND progman = FindWindow(L"Progman", nullptr);
    SendMessageTimeout(progman, 0x052C, 0, 0,
                       SMTO_NORMAL, 1000, nullptr);
    HWND workerw = nullptr;
    Sleep(100);
    EnumWindows([](HWND top, LPARAM p) -> BOOL
                {
        wchar_t cls[64]; GetClassName(top, cls, _countof(cls));
        if (wcscmp(cls, L"WorkerW") == 0) {
            HWND pShell = FindWindowEx(top, nullptr, L"SHELLDLL_DefView", nullptr);
            if (pShell) {
                *reinterpret_cast<HWND*>(p) = top;
                return FALSE;
            }
        }
        return TRUE; }, reinterpret_cast<LPARAM>(&workerw));

    WNDCLASS wc = {sizeof(wc), WndProc, 0, 0, hInst, nullptr,
                   LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr,
                   L"WebView2CompSample"};
    RegisterClass(&wc);
    g_hostHwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_NOACTIVATE,
        wc.lpszClassName, nullptr,
        WS_POPUP,
        0, 0,
        500,
        500,
        workerw, nullptr, hInst, nullptr);

    ShowWindow(g_hostHwnd, SW_SHOW);
    SetLayeredWindowAttributes(g_hostHwnd, 0, 255, LWA_ALPHA);

    // Initialize DirectComposition and WebView2
    InitDCompPipeline();
    CreateWebView2Composition();

    // Standard message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void InitDCompPipeline()
{
    // Create D3D11 device with BGRA support
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &g_d3dDevice,
        nullptr, nullptr);

    // Query IDXGIDevice
    g_d3dDevice.As(&g_dxgiDevice);

    // Create DirectComposition device
    DCompositionCreateDevice(
        g_dxgiDevice.Get(),
        __uuidof(IDCompositionDevice),
        &g_dcompDevice);

    // Create a composition target tied to our HWND
    g_dcompDevice->CreateTargetForHwnd(
        g_hostHwnd,
        TRUE,
        &g_dcompTarget);

    // Create root visual
    g_dcompDevice->CreateVisual(&g_dcompVisual);
    g_dcompTarget->SetRoot(g_dcompVisual.Get());
    g_dcompDevice->Commit();
}

void CreateWebView2Composition()
{
    // Create WebView2 environment
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT envResult, ICoreWebView2Environment *env) -> HRESULT
            {
                // Query for Env3 to access composition API
                ComPtr<ICoreWebView2Environment3> env3;
                env->QueryInterface(IID_PPV_ARGS(&env3));
                if (!env3)
                    return E_FAIL;

                // Create composition controller
                env3->CreateCoreWebView2CompositionController(
                    g_hostHwnd,
                    Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
                        [](HRESULT ctrlResult, ICoreWebView2CompositionController *ctrl) -> HRESULT
                        {
                            g_compController = ctrl; // Keep alive

                            // Obtain regular controller interface
                            ComPtr<ICoreWebView2Controller> viewCtrl;
                            ctrl->QueryInterface(IID_PPV_ARGS(&viewCtrl));

                            // Attach the DirectComposition visual
                            ctrl->put_RootVisualTarget(g_dcompVisual.Get());
                            // Make WebView visible
                            viewCtrl->put_IsVisible(TRUE);

                            // Resize to fill host
                            RECT rc;
                            GetClientRect(g_hostHwnd, &rc);
                            viewCtrl->put_Bounds(rc);

                            // Commit composition tree
                            g_dcompDevice->Commit();

                            // Navigate to a test page
                            ComPtr<ICoreWebView2> webview;
                            viewCtrl->get_CoreWebView2(&webview);
                            webview->Navigate(L"https://www.google.com");

                            return S_OK;
                        })
                        .Get());
                return S_OK;
            })
            .Get());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_compController)
        {
            ComPtr<ICoreWebView2Controller> viewCtrl;
            g_compController.As(&viewCtrl);
            RECT rc;
            GetClientRect(hWnd, &rc);
            viewCtrl->put_Bounds(rc);
            g_dcompDevice->Commit();
        }
        break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        ForwardMouse(msg, wParam, lParam);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ForwardMouse(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!g_compController)
        return;

    COREWEBVIEW2_MOUSE_EVENT_KIND kind;
    switch (msg)
    {
    case WM_MOUSEMOVE:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE;
        break;
    case WM_LBUTTONDOWN:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN;
        break;
    case WM_LBUTTONUP:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP;
        break;
    default:
        return;
    }

    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    g_compController->SendMouseInput(
        kind,
        COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE,
        0,
        pt);
}
