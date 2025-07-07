#pragma once

#include <windows.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

void HandleMediaPropertiesRequest(HWND hwnd, json msg);
void HandlePlaybackInfoRequest(HWND hwnd, json msg);
void HandleTimelinePropertiesRequest(HWND hwnd, json msg);
void HandleThumbnailRequest(HWND hwnd, json msg);
void AddMediaSubscription(HWND hwnd, json msg);
void RemoveMediaSubscription(HWND hwnd);
void AddPlaybackSubscription(HWND hwnd, json msg);
void RemovePlaybackSubscription(HWND hwnd);
void AddTimelineSubscription(HWND hwnd, json msg);
void RemoveTimelineSubscription(HWND hwnd);
void SubscriptionCleanup(HWND hwnd);
void SendMediaCommand(std::string const &cmd);
void SetPlaybackPosition(int position);