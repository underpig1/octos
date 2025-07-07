#pragma once

#include <windows.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

void HandleMediaPropertiesRequest(HWND hwnd, json msg);
void HandlePlaybackInfoRequest(HWND hwnd, json msg);
void HandleTimelinePropertiesRequest(HWND hwnd, json msg);