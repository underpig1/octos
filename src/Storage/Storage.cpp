#include <Shlwapi.h>
#include <filesystem>
#include <vector>
#include <shlobj.h>
#include <shobjidl.h>
#include <comdef.h>
#include <iostream>
#include <fstream>

#include "Storage.h"
#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
std::vector<ConfigParams> allParams;

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

std::wstring ResolvePath(std::wstring relativePath, bool includeFileScheme)
{
    std::wstring exePath = GetAppPath();
    std::wstring fullPath = CombinePaths(exePath, relativePath);
    std::wstring url = fullPath;
    if (includeFileScheme)
        url = L"file:///" + fullPath;
    return url;
}

std::wstring GetWallpapersDir()
{
    return ResolvePath(L"wallpapers");
}

bool InstallWallpaper(const std::wstring &zipPath)
{
    wprintf(L"[Storage] Unzipping\n");
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    IShellDispatch *shell = nullptr;
    Folder *zipFolder = nullptr;
    Folder *destFolderObj = nullptr;
    bool success = false;
    const std::wstring destFolder = GetWallpapersDir();
    fs::create_directories(destFolder);

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

        wprintf(L"[Storage] ZIP path in VARIANT : % s\n ", vZip.bstrVal);
        wprintf(L"[Storage] zipFolder: %s\n", vDest.bstrVal);
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
                    wprintf(L"[Storage] UNZIPPED SUCCESSFULLY\n");
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
        std::string author = j.value("name", "");
        std::string name = j.value("name", "");
        std::string description = j.value("description", "");
        std::string imagePath = j.value("imagePath", "");
        params.author = std::wstring(author.begin(), author.end());
        params.name = std::wstring(name.begin(), name.end());
        params.description = std::wstring(description.begin(), description.end());
        params.imagePath = ResolvePath(CombinePaths(path.parent_path(), std::wstring(imagePath.begin(), imagePath.end())), true);
        if (j.contains("options") && j["options"].is_object())
        {
            std::string options = j["options"].dump();
            params.options = std::wstring(options.begin(), options.end());
        }
        return params;
    }
    return params;
}

std::vector<ConfigParams> IterateWallpapersDir()
{
    std::vector<ConfigParams> allParams;
    std::wstring path = GetWallpapersDir();
    if (path.c_str())
        for (const auto &entry : fs::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                std::wstring name = entry.path().filename().wstring();
                fs::path configPath = entry.path() / L"octos.json";
                ConfigParams params;
                if (fs::exists(configPath))
                {
                    params = ReadConfig(configPath);
                    params.configPath = configPath.wstring();
                    if (params.name == L"")
                        params.name = name;
                    else
                        name = params.name;
                }
                else
                    params.name = name;
                params.folderPath = entry.path().wstring();

                allParams.push_back(params);
            }
        }
    return allParams;
}

std::wstring ParamsAsJsonString(ConfigParams p)
{
    std::string jsonString = "{"
                             "\"author\":" +
                             json(p.author).dump() + ","
                                                     "\"name\":" +
                             json(p.name).dump() + ","
                                                   "\"description\":" +
                             json(p.description).dump() + ","
                                                          "\"imagePath\":" +
                             json(p.imagePath).dump() + ","
                                                      "\"options\":" +
                             json(p.options).dump() +
                             "}";
    return std::wstring(jsonString.begin(), jsonString.end());
}