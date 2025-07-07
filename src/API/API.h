#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void RespondToHwnd(HWND hwnd, json msg, json data);
void HandleRequest(json msg, HWND hwnd);
void HandleCommand(json msg, HWND hwnd);
void HandleSubscription(json msg, HWND hwnd);