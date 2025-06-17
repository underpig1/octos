#include <Shlwapi.h>

#include "Storage.h"

std::wstring GetAppPath()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
    return std::wstring(exePath);
}

std::wstring ResolvePath(std::wstring relativePath, bool includeFileScheme = false)
{
    std::wstring exePath = GetAppPath();
    std::wstring fullPath = exePath + L"\\" + relativePath;
    for (auto &c : fullPath)
        if (c == L'\\')
            c = L'/';
    std::wstring url = fullPath;
    if (includeFileScheme)
        url = L"file:///" + fullPath;
    return url;
}