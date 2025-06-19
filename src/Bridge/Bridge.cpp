#include <winrt/Windows.Data.Json.h>

#include "Bridge.h"
#include "../main.h"
#include "../Core/Core.h"
#include "../WebView/Webview.h"

using namespace winrt::Windows::Data::Json;

void HandleWebMessage(std::wstring msg, HWND hwnd)
{
    try
    {
        wprintf(L"RECEIVED JSON: %s\n", msg.c_str());
        JsonObject json = JsonObject::Parse(msg);
        winrt::hstring type = json.GetNamedString(L"type");
        wprintf(L"TYPE: %ls\n", type.c_str());

        if (type == L"drag")
        {
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        else if (type == L"minimize")
            ShowWindow(app_hwnd, SW_MINIMIZE);
        else if (type == L"maximize")
            if (IsZoomed(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
            else
                ShowWindow(hwnd, SW_MAXIMIZE);
        else if (type == L"close")
            ShowWindow(app_hwnd, SW_HIDE);
        else if (type == L"open-external-link")
        {
            winrt::hstring url = json.GetNamedString(L"url");
            if (url.c_str())
                ShellExecute(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        else if (type == L"refresh")
            RecreateWallpapers();
        else {
            wprintf(L"UNKNOWN TYPE: %ls\n", type.c_str());
        }
    }
    catch (const std::exception& e)
    {
        wprintf(L"EXCEPTION: %S\n", e.what());
    }
    catch (...)
    {
        wprintf(L"UNKNOWN EXCEPTION\n");
    }
}

void DispatchJSON(std::wstring message)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(app_hwnd, GWLP_USERDATA));
    if (data && data->webview)
        data->webview->PostWebMessageAsJson(message.c_str());
}

void RaiseErrorBox(std::wstring title, std::wstring caption)
{
    std::wstring message =
        L"{\"type\":\"error-box\",\"title\":\"" + title +
        L"\",\"caption\":\"" + caption +
        L"\"}";
    DispatchJSON(message);
}