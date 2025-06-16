#include <winrt/Windows.Data.Json.h>

#include "Bridge.h"

using namespace winrt::Windows::Data::Json;

void HandleWebMessage(std::wstring msg, HWND hwnd)
{
    try
    {
        JsonObject json = JsonObject::Parse(msg);
        winrt::hstring type = json.GetNamedString(L"type");
        if (type == L"drag")
        {
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        else if (type == L"resize")
        {
            int width = static_cast<int>(json.GetNamedNumber(L"width"));
            int height = static_cast<int>(json.GetNamedNumber(L"height"));
        }
    }
    catch (...)
    {}
}