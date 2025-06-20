#include <nlohmann/json.hpp>

#include "Bridge.h"
#include "../main.h"
#include "../Core/Core.h"
#include "../WebView/Webview.h"
#include "../Storage/Storage.h"

using json = nlohmann::json;

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

void HandleWebMessage(std::wstring msg, HWND hwnd)
{
    try
    {
        json j = json::parse(to_string(msg));
        std::string type = j.value("type", "");

        if (type == "drag")
        {
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        else if (type == "minimize")
            ShowWindow(app_hwnd, SW_MINIMIZE);
        else if (type == "maximize")
            if (IsZoomed(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
            else
                ShowWindow(hwnd, SW_MAXIMIZE);
        else if (type == "close")
            ShowWindow(app_hwnd, SW_HIDE);
        else if (type == "open-external-link")
        {
            std::string url = j.value("url", "");
            if (!url.empty())
            {
                if (url.c_str())
                    ShellExecute(NULL, L"open", to_wstring(url).c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
        }
        else if (type == "refresh")
            RecreateWallpapers();
        else if (type == "request-wallpaper-data")
        {
            std::wstring message = IterateWallpapersAsJsonString();
            DispatchJSON(message);
        }
        else if (type == "wallpaper-file-picker")
            SelectAndInstallWallpaper();
        else if (type == "request-visibility")
        {
            std::wstring visibility = IsWindowVisible(ms[0].hwnd) ? L"true" : L"false";
            std::wstring message = L"{\"type\":\"visibility\",\"value\":" + visibility + L"}";
            DispatchJSON(message);
        }
        else if (type == "set-visibility")
        {
            if (j.contains("value") && j["value"].is_boolean())
            {
                bool visibility = j["value"];
                SetWallpaperVisibility(visibility);
            }
        }
        else
        {
            wprintf(L"UNKNOWN TYPE\n");
        }
    }
    catch (const std::exception &e)
    {
        wprintf(L"EXCEPTION: %S\n", e.what());
    }
    catch (...)
    {
        wprintf(L"UNKNOWN EXCEPTION\n");
    }
}