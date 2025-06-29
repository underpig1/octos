#include <Shlwapi.h>
#include <filesystem>
#include <vector>
#include <shlobj.h>
#include <shobjidl.h>
#include <comdef.h>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <urlmon.h>

#include "Storage.h"
#include "../Bridge/Bridge.h"
#include "../main.h"

namespace fs = std::filesystem;
using json = nlohmann::json;
std::vector<ConfigParams> allParams;

std::wstring to_wstring(const std::string &utf8str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8str);
}

std::string to_string(const std::wstring &utf16str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(utf16str);
}

std::wstring GetAppPath()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
    return std::wstring(exePath);
}

std::wstring NormalizePath(std::wstring path)
{
    for (auto &ch : path)
        if (ch == L'/')
            ch = L'\\';
    return path;
}

std::wstring CombinePaths(std::wstring firstPath, std::wstring secondPath)
{
    return NormalizePath(firstPath + L"\\" + secondPath);
}

std::wstring AddFileScheme(std::wstring path)
{
    return L"file:///" + path;
}

std::wstring ResolvePath(std::wstring relativePath, bool includeFileScheme)
{
    std::wstring exePath = GetAppPath();
    std::wstring fullPath = CombinePaths(exePath, relativePath);
    std::wstring url = fullPath;
    if (includeFileScheme)
        url = AddFileScheme(fullPath);
    return url;
}

std::wstring GetWallpapersDir()
{
    return ResolvePath(L"wallpapers");
}

bool InstallWallpaper(const std::wstring &zipPath)
{
    wprintf(L"[Storage] Installing from: %s\n", zipPath.c_str());
    std::wstring destFolder = GetWallpapersDir();
    fs::create_directories(destFolder);

    if (fs::exists(zipPath) && fs::path(zipPath).extension() == L".zip")
    {
        wprintf(L"[Storage] Unzipping\n");
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        IShellDispatch *shell = nullptr;
        Folder *zipFolder = nullptr;
        Folder *destFolderObj = nullptr;
        bool success = false;

        if (SUCCEEDED(CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shell))))
        {
            VARIANT vZip, vDest;
            VariantInit(&vZip);
            VariantInit(&vDest);

            vZip.vt = VT_BSTR;
            vZip.bstrVal = SysAllocString(NormalizePath(zipPath).c_str());
            vDest.vt = VT_BSTR;
            vDest.bstrVal = SysAllocString(NormalizePath(destFolder).c_str());

            shell->NameSpace(vZip, &zipFolder);
            shell->NameSpace(vDest, &destFolderObj);

            if (zipFolder && destFolderObj)
            {
                FolderItems *items = nullptr;
                if (SUCCEEDED(zipFolder->Items(&items)))
                {
                    VARIANT vItems;
                    VariantInit(&vItems);
                    vItems.vt = VT_DISPATCH;
                    vItems.pdispVal = items;

                    VARIANT vOpt;
                    VariantInit(&vOpt);
                    vOpt.vt = VT_I4;
                    vOpt.lVal = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;

                    HRESULT hr = destFolderObj->CopyHere(vItems, vOpt);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"[Storage] Unzipped successfully\n");
                        success = true;
                    }

                    VariantClear(&vItems);
                    items->Release();
                }
            }

            if (zipFolder)
                zipFolder->Release();
            if (destFolderObj)
                destFolderObj->Release();
            SysFreeString(vZip.bstrVal);
            SysFreeString(vDest.bstrVal);
            shell->Release();
        }

        CoUninitialize();
        return success;
    }

    wprintf(L"[Storage] Unsupported input path: not a .zip or directory\n");
    return false;
}

void DownloadWallpaper(const std::wstring url)
{
    wchar_t tempPath[MAX_PATH];
    DWORD pathLen = GetTempPathW(MAX_PATH, tempPath);
    fs::path zipPath = fs::path(tempPath) / fs::path(url).filename();
    HRESULT hr = URLDownloadToFileW(nullptr, url.c_str(), zipPath.c_str(), 0, nullptr);
    if (SUCCEEDED(hr))
    {
        if (!InstallWallpaper(zipPath))
            RaiseErrorBox(L"Download failed", L"Please try again.");
        fs::remove(zipPath);
    }
    else
    {
        RaiseErrorBox(L"Download failed", L"Please try again.");
    }
}

bool ReadJsonFile(const fs::path &filePath, json &out)
{
    if (!fs::exists(filePath))
        return false;
    std::ifstream inFile(filePath);
    if (!inFile.is_open())
        return false;
    try
    {
        inFile >> out;
        return true;
    }
    catch (const json::parse_error &e)
    {
        return false;
    }
}

ConfigParams ReadConfig(fs::path path)
{
    json j;
    ConfigParams params;
    if (ReadJsonFile(path, j))
    {
        std::string author = j.value("author", "");
        std::string name = j.value("name", "");
        std::string description = j.value("description", "");
        params.author = to_wstring(author);
        params.name = to_wstring(name);
        params.description = to_wstring(description);
        if (j.contains("image"))
        {
            std::string image = j.value("image", "");
            std::wstring imagePath = path.parent_path() / NormalizePath(to_wstring(image));
            if (fs::exists(imagePath))
                params.imagePath = AddFileScheme(imagePath);
        }
        if (j.contains("entry"))
        {
            std::string entryPath = j.value("entry", "");
            if (entryPath.rfind("http://", 0) == 0 || entryPath.rfind("https://", 0) == 0)
                params.entryPath = to_wstring(entryPath);
            else
            {
                std::wstring normalEntryPath = path.parent_path() / NormalizePath(to_wstring(entryPath));
                if (fs::exists(normalEntryPath))
                    params.entryPath = AddFileScheme(normalEntryPath);
            }
        }
        if (j.contains("options") && j["options"].is_object())
        {
            std::string options = j["options"].dump();
            params.options = to_wstring(options);
        }
        else if (j.contains("prefs") && j["prefs"].is_object())
        {
            std::string options = j["prefs"].dump();
            params.options = to_wstring(options);
        }
        return params;
    }
    return params;
}

std::vector<ConfigParams> IterateWallpapersDir()
{
    std::vector<ConfigParams> allParams;
    std::wstring path = GetWallpapersDir();
    if (!path.empty())
        for (const auto &entry : fs::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                std::wstring name = entry.path().filename().wstring();
                fs::path configPath = entry.path() / L"octos.json";
                ConfigParams params;
                if (!fs::exists(configPath))
                    configPath = entry.path() / L"mod.json";
                if (fs::exists(configPath))
                {
                    params = ReadConfig(configPath);
                    params.configPath = configPath.wstring();
                    if (params.name.empty())
                        params.name = name;
                    else
                        name = params.name;
                    if (params.entryPath.empty())
                    {
                        std::wstring entryCandidate = L"";
                        for (const auto &file : fs::directory_iterator(entry))
                        {
                            if (file.is_regular_file())
                            {
                                if (file.path().extension() == L".html")
                                {
                                    if (!entryCandidate.empty())
                                        entryCandidate = file.path().filename().wstring();
                                    if (file.path().filename() == L"index.html")
                                        break;
                                }
                            }
                        }
                        if (entryCandidate.empty())
                            params.entryPath = L"";
                        else
                            params.entryPath = AddFileScheme(entry / NormalizePath(entryCandidate));
                    }
                    params.folderPath = entry.path().wstring();
                }
                if (!params.entryPath.empty())
                    allParams.push_back(params);
            }
        }
    return allParams;
}

std::wstring ParamsAsJsonString(ConfigParams p)
{
    json j = {
        {"author", to_string(p.author)},
        {"name", to_string(p.name)},
        {"description", to_string(p.description)},
        {"folderPath", to_string(p.folderPath)},
        {"imagePath", to_string(p.imagePath)},
        {"entryPath", to_string(p.entryPath)},
        {"options", json::parse(to_string(p.options))}};
    std::string dump = j.dump();
    return to_wstring(dump);
}

std::wstring IterateWallpapersAsJsonString()
{
    std::wstring jsonString = L"{\"type\":\"wallpaper-data\",\"data\":[ ";
    std::vector<ConfigParams> allParams = IterateWallpapersDir();
    for (auto &p : allParams)
    {
        std::wstring pString = ParamsAsJsonString(p) + L",";
        jsonString.append(pString);
    }
    jsonString.pop_back();
    jsonString.append(L"]}");
    wprintf(L"%s", jsonString.c_str());
    return jsonString;
}

void SelectAndInstallWallpaper()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    std::wstring result;
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pFileOpen = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void **>(&pFileOpen));
        if (SUCCEEDED(hr))
        {
            COMDLG_FILTERSPEC zipFilter[] = {
                {L"ZIP Archives", L"*.zip"},
                {L"All Files", L"*.*"}};
            pFileOpen->SetFileTypes(ARRAYSIZE(zipFilter), zipFilter);
            pFileOpen->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST);

            HRESULT hr = pFileOpen->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem *pItem;
                if (SUCCEEDED(pFileOpen->GetResult(&pItem)))
                {
                    PWSTR pszFilePath = nullptr;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                    {
                        result = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                return;
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    bool succeeded = false;
    if (!result.empty())
    {
        fs::path filePath = result;
        if (fs::exists(filePath) && filePath.extension() == L".zip")
            succeeded = InstallWallpaper(filePath);
    }
    if (!succeeded)
        RaiseErrorBox(L"Wallpaper failed to add", L"Please try again.");
}

void RemoveWallpaper(std::wstring folderPath)
{
    std::error_code ec;
    fs::remove_all(folderPath, ec);
    if (ec)
        RaiseErrorBox(L"Failed to uninstall", L"Please try again.");
}

std::wstring GetPrefsPath()
{
    return ResolvePath(L"preferences.json");
}

json LoadPrefs()
{
    const fs::path prefsPath = GetPrefsPath();
    if (fs::exists(prefsPath))
    {
        std::ifstream in(prefsPath);
        if (!in)
            return json{};
        try
        {
            json prefs;
            in >> prefs;
            return prefs;
        }
        catch (const json::parse_error &)
        {
            return json{};
        }
    }
    return json{};
}

std::wstring LoadPrefsAsJsonString()
{
    json prefs = LoadPrefs();
    return L"{\"type\":\"prefs\",\"data\":" + to_wstring(prefs.dump()) + L"}";
}

bool AddToStartup()
{
    const std::wstring &exePath = GetAppPath();
    HKEY hKey;
    const wchar_t *runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return false;
    LONG result = RegSetValueExW(
        hKey,
        CLASS_NAME,
        0,
        REG_SZ,
        reinterpret_cast<const BYTE *>(exePath.c_str()),
        static_cast<DWORD>((exePath.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RemoveFromStartup()
{
    HKEY hKey;
    const wchar_t *runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return false;
    LONG result = RegDeleteValueW(hKey, CLASS_NAME);
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}

void HandlePrefsChange(const json prefs)
{
    if (prefs.contains("appOptions") && prefs["appOptions"].is_object())
    {
        const auto &appOptions = prefs["appOptions"];
        auto setPrefOnChange = [&](const char *key, Pref prefId, std::function<void(bool result)> onChanged = {})
        {
            if (appOptions.contains(key) && appOptions[key].is_boolean())
            {
                bool value = appOptions[key];
                if (GetPref(prefId) != value)
                {
                    SetPref(prefId, value);
                    if (onChanged)
                        onChanged(value);
                }
            }
        };
        setPrefOnChange("disableMouseInput", Pref::DisableMouseInput);
        setPrefOnChange("memorySaver", Pref::MemorySaver);
        setPrefOnChange("runOnStartup", Pref::RunOnStartup, [](bool result) {
            if (result) AddToStartup();
            else RemoveFromStartup();
        });
    }
}

void DumpPrefs(const json prefs)
{
    const fs::path prefsPath = GetPrefsPath();
    std::ofstream out(prefsPath);
    if (!out)
        return;
    out << prefs.dump();
    HandlePrefsChange(prefs);
}