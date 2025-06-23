#pragma once

#include <string>
#include <nlohmann/json.hpp>

std::wstring ResolvePath(std::wstring relativePath, bool includeFileScheme = false);
bool InstallWallpaper(const std::wstring &inputPath);
std::wstring IterateWallpapersAsJsonString();
void SelectAndInstallWallpaper();
std::wstring to_wstring(const std::string &utf8str);
std::string to_string(const std::wstring &utf16str);
std::wstring LoadPrefsAsJsonString();
void DumpPrefs(const nlohmann::json prefs);
void RemoveWallpaper(std::wstring folderPath);

struct ConfigParams
{
    std::wstring author;
    std::wstring name;
    std::wstring description;
    std::wstring folderPath;
    std::wstring configPath;
    std::wstring imagePath;
    std::wstring entryPath;
    std::wstring options;
};