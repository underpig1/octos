#pragma once

#include <windows.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

bool RouteArbitraryMediaRequest(HWND hwnd, json msg, std::string key);