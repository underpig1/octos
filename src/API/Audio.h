#pragma once

#include <windows.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void AddAudioSubscription(HWND hwnd, json msg);
void RemoveAudioSubscription(HWND hwnd);