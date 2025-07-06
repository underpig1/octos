#include <windows.h>
#include <iostream>
#include <shlobj.h>
#include <nlohmann/json.hpp>

#include "../Bridge/Bridge.h"
#include "../Storage/Storage.h"

using json = nlohmann::json;

void SetDesktopIconsVisibility(BOOL show)
{
    SHELLSTATE shellState = {};
    SHGetSetSettings(&shellState, SSF_HIDEICONS, FALSE);
    shellState.fHideIcons = !show;
    SHGetSetSettings(&shellState, SSF_HIDEICONS, TRUE);
    HWND progman = FindWindow(L"Progman", nullptr);
    if (progman)
        PostMessage(progman, WM_COMMAND, 41504, 0);
}

void HandleRequest(json data, HWND hwnd)
{
    if (data.contains("requestId") && data.contains("requestType"))
    {
        json response = {
            "type", "response",
            "requestId", data["requestId"],
            "requestType", data["requestType"],

        };
    }
}

void HandleCommand(json data, HWND hwnd)
{

}