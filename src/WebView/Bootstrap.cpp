#include <urlmon.h>
#include <string>
#include <atomic>
#include <CommCtrl.h>
#include <WebView2.h>
#include <Uxtheme.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "UxTheme.lib")

#include "../main.h"
#include "Bootstrap.h"
#include "../Storage/Storage.h"

const std::wstring bootstrapUrl = L"https://go.microsoft.com/fwlink/p/?LinkId=2124703";

HWND hostHwnd;
HWND labelHwnd;
HWND progressHwnd;

std::wstring GetBootstrapTargetPath()
{
    wchar_t tempPath[MAX_PATH] = {0};
    DWORD length = GetTempPathW(MAX_PATH, tempPath);
    if (length == 0 || length > MAX_PATH)
        return L"";
    return std::wstring(tempPath) + L"WebView2Bootstrapper.exe";
}

bool IsWebView2Installed()
{
    PWSTR versionInfo = nullptr;
    HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(nullptr, &versionInfo);
    if (SUCCEEDED(hr) && versionInfo != nullptr)
    {
        CoTaskMemFree(versionInfo);
        return true;
    }
    return false;
}

void AddBootstrapLink()
{
    wprintf(L"\nCREATING LINK");
    if (progressHwnd && IsWindow(progressHwnd))
        DestroyWindow(progressHwnd);

    SetWindowText(labelHwnd, L"Octos requires the WebView2 Runtime. Download the runtime below, then relaunch Octos.");

    HWND hwndButton = CreateWindowEx(
        0, L"BUTTON", L"Download Runtime",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        100, 50, 100, 25,
        hostHwnd, (HMENU)1234, g_hInstance, nullptr);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(hwndButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    UpdateWindow(hostHwnd);
}

void OpenBootstrapLink()
{
    ShellExecute(nullptr, L"open", bootstrapUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

LRESULT CALLBACK HostProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
    {
        if (LOWORD(wParam) == 1234 && HIWORD(wParam) == BN_CLICKED)
            OpenBootstrapLink();
        break;
    }
    case WM_USER + 1:
        AddBootstrapLink();
        break;
    case WM_USER + 2:
        if (hostHwnd)
        {
            DestroyWindow(hostHwnd);
            hostHwnd = nullptr;
            labelHwnd = nullptr;
            progressHwnd = nullptr;
        }
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool CreateProgressWindow()
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = HostProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = L"OctosSetup";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = g_hIcon;
    RegisterClassEx(&wc);

    RECT rc = {0, 0, 300, 90};
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    hostHwnd = CreateWindowEx(
        0, L"OctosSetup",
        L"Octos Setup",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, g_hInstance, nullptr);

    if (!hostHwnd)
        return false;

    labelHwnd = CreateWindowEx(
        0, L"STATIC", L"Setting up Octos for the first time...",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 15, 260, 30,
        hostHwnd, nullptr, g_hInstance, nullptr);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(labelHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

    INITCOMMONCONTROLSEX icc = {sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS};
    InitCommonControlsEx(&icc);

    progressHwnd = CreateWindowEx(
        0, PROGRESS_CLASS, nullptr,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH | PBS_MARQUEE,
        20, 45, 260, 20,
        hostHwnd, nullptr, g_hInstance, nullptr);

    SendMessage(progressHwnd, PBM_SETMARQUEE, TRUE, 0);

    ShowWindow(hostHwnd, SW_SHOW);
    UpdateWindow(hostHwnd);

    return true;
}

class ProgressDialog : public IBindStatusCallback
{
public:
    ProgressDialog(HWND hwnd) : m_hwndProgress(hwnd), m_refCount(1) {}
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        if (riid == IID_IUnknown || riid == IID_IBindStatusCallback)
        {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG)
    AddRef() { return InterlockedIncrement(&m_refCount); }
    STDMETHODIMP_(ULONG)
    Release()
    {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0)
            delete this;
        return count;
    }
    STDMETHODIMP OnStartBinding(DWORD, IBinding *) { return E_NOTIMPL; }
    STDMETHODIMP GetPriority(LONG *) { return E_NOTIMPL; }
    STDMETHODIMP OnLowResource(DWORD) { return E_NOTIMPL; }
    STDMETHODIMP OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG, LPCWSTR)
    {
        if (ulProgressMax > 0 && m_hwndProgress)
        {
            int percent = static_cast<int>((ulProgress * 100) / ulProgressMax);
            SendMessage(m_hwndProgress, PBM_SETPOS, percent, 0);
        }
        return S_OK;
    }
    STDMETHODIMP OnStopBinding(HRESULT, LPCWSTR) { return S_OK; }
    STDMETHODIMP GetBindInfo(DWORD *, BINDINFO *) { return E_NOTIMPL; }
    STDMETHODIMP OnDataAvailable(DWORD, DWORD, FORMATETC *, STGMEDIUM *) { return E_NOTIMPL; }
    STDMETHODIMP OnObjectAvailable(REFIID, IUnknown *) { return E_NOTIMPL; }

private:
    HWND m_hwndProgress;
    LONG m_refCount;
};

HRESULT DownloadWebView2Installer(const std::wstring &url, const std::wstring &destPath)
{
    ProgressDialog *pCallback = new ProgressDialog(progressHwnd);
    IMoniker *pMoniker = nullptr;
    IBindCtx *pBindCtx = nullptr;

    HRESULT hr = CreateBindCtx(0, &pBindCtx);
    if (FAILED(hr))
    {
        pCallback->Release();
        return hr;
    }

    hr = CreateURLMoniker(nullptr, url.c_str(), &pMoniker);
    if (FAILED(hr))
    {
        pBindCtx->Release();
        pCallback->Release();
        return hr;
    }

    hr = URLDownloadToFile(pBindCtx, url.c_str(), destPath.c_str(), 0, pCallback);

    pMoniker->Release();
    pBindCtx->Release();
    pCallback->Release();

    return hr;
}

bool InstallWebView2(const std::wstring &installerPath, HWND hwndProgress)
{
    SendMessage(hwndProgress, PBM_SETMARQUEE, TRUE, 0);

    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    wchar_t cmdLineArgs[] = L"/silent /install";
    if (CreateProcess(installerPath.c_str(), cmdLineArgs, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        SendMessage(hwndProgress, PBM_SETMARQUEE, FALSE, 0);
        return true;
    }
    SendMessage(hwndProgress, PBM_SETMARQUEE, FALSE, 0);
    return false;
}

bool RunBootstrapProcess()
{
    std::wstring bootstrapPath = GetBootstrapTargetPath();
    if (bootstrapPath.empty())
        return false;
    if (FAILED(DownloadWebView2Installer(bootstrapUrl, bootstrapPath)))
        return false;
    if (!InstallWebView2(bootstrapPath, progressHwnd))
        return false;
    return true;
}

void HandleBootstrap(std::function<void()> callback)
{
    if (IsWebView2Installed()) {
        callback();
        return;
    }
    if (!CreateProgressWindow()) {
        int result = MessageBox(nullptr, L"Cannot open Octos", L"Fatal error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        PostQuitMessage(0);
        return;
    }
    std::thread bootstrapThread([callback]()
                                {
            bool result = RunBootstrapProcess();
            if (!result)
            {
                PostMessage(hostHwnd, WM_USER + 1, 0, 0); // add download bootstrapper link
                return;
            }
            PostMessage(hostHwnd, WM_USER + 2, 0, 0); // cleanup
            RestartApp(); });
    bootstrapThread.detach();
}
