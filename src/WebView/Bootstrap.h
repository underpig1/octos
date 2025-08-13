#pragma once

#include <windows.h>
#include <functional>

void HandleBootstrap(std::function<void()> callback);