#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void RespondToHwnd(HWND hwnd, json msg, json data, bool mainThread = false);
void SendEventToHwnd(HWND hwnd, json msg, json data, bool mainThread = false);
void HandleRequest(json msg, HWND hwnd);
void HandleCommand(json msg, HWND hwnd);
void HandleSubscription(json msg, HWND hwnd, bool sub);