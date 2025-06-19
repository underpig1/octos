#pragma once

#include <string>

std::wstring ResolvePath(std::wstring relativePath, bool includeFileScheme = false);
bool InstallWallpaper(const std::wstring &zipPath);

struct ConfigParams
{
    std::wstring author;
    std::wstring name;
    std::wstring description;
    std::wstring folderPath;
    std::wstring configPath;
    std::wstring imagePath;
    std::wstring options;
};