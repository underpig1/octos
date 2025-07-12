#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

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

std::wstring ResolvePath(std::wstring relativePath, bool includeFileScheme = false);
bool InstallWallpaper(const std::wstring &inputPath);
std::wstring IterateWallpapersAsJsonString();
bool SelectAndInstallWallpaper();
std::wstring to_wstring(const std::string &utf8str);
std::string to_string(const std::wstring &utf16str);
std::wstring LoadPrefsAsJsonString();
void DumpPrefs(const nlohmann::json prefs);
void RemoveWallpaper(std::wstring folderPath);
bool DownloadWallpaper(const std::wstring url);
void LoadAndHandleAppPrefs();
std::wstring SelectFolderAndGetConfigAsJsonString();
std::wstring GetConfigFromFolderAsJsonString(std::wstring dirPath);
void DumpConfig(std::wstring path, json data);

    extern std::vector<ConfigParams> g_allParams;