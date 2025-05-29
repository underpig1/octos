#include "Dispatch.h"
#include "../Core/Core.h"

void Dispatch(HWND hwnd, LPCWSTR json)
{
    if (IsWindow(hwnd))
    {
        WebViewData *data = GetWebViewData(hwnd);
        if (data)
        {
            data->webview->PostWebMessageAsJson(json);
        }
    }
}

void DispatchToAllWindows(LPCWSTR json)
{
    for (auto& mw : ms)
    {
        Dispatch(mw.hwnd, json);
    }
}