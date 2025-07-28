#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "API.h"

using Microsoft::WRL::ComPtr;
using json = nlohmann::json;

std::unordered_map<HWND, json> audioSubscriptions;
std::mutex subscriptionsMutex;

std::atomic<bool> capturing{false};
std::thread captureThread;

std::vector<float> ConvertPCM16ToFloat(const BYTE *data, UINT32 numFrames, int numChannels)
{
    std::vector<float> result(numFrames * numChannels);
    const int16_t *samples = reinterpret_cast<const int16_t *>(data);
    for (UINT32 i = 0; i < numFrames * numChannels; i++)
        result[i] = samples[i] / 32768.0f;
    return result;
}

void CaptureAudioLoop()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        std::cerr << "CoInitializeEx failed\n";
        capturing = false;
        return;
    }

    ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create IMMDeviceEnumerator\n";
        capturing = false;
        CoUninitialize();
        return;
    }

    ComPtr<IMMDevice> defaultDevice;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get default audio endpoint\n";
        capturing = false;
        CoUninitialize();
        return;
    }

    ComPtr<IAudioClient> audioClient;
    hr = defaultDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&audioClient);
    if (FAILED(hr))
    {
        std::cerr << "Failed to activate audio client\n";
        capturing = false;
        CoUninitialize();
        return;
    }

    WAVEFORMATEX *waveFormat = nullptr;
    hr = audioClient->GetMixFormat(&waveFormat);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get mix format\n";
        capturing = false;
        CoUninitialize();
        return;
    }

    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        10000000, // 1 second buffer
        0,
        waveFormat,
        NULL);
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize audio client\n";
        CoTaskMemFree(waveFormat);
        capturing = false;
        CoUninitialize();
        return;
    }

    ComPtr<IAudioCaptureClient> captureClient;
    hr = audioClient->GetService(IID_PPV_ARGS(&captureClient));
    if (FAILED(hr))
    {
        std::cerr << "Failed to get audio capture client\n";
        CoTaskMemFree(waveFormat);
        capturing = false;
        CoUninitialize();
        return;
    }

    hr = audioClient->Start();
    if (FAILED(hr))
    {
        std::cerr << "Failed to start audio client\n";
        CoTaskMemFree(waveFormat);
        capturing = false;
        CoUninitialize();
        return;
    }

    auto lastSendTime = std::chrono::steady_clock::now();

    while (capturing)
    {
        UINT32 packetLength = 0;
        hr = captureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr))
            break;

        if (packetLength == 0)
        {
            Sleep(10);
            continue;
        }

        BYTE *data;
        UINT32 numFramesAvailable;
        DWORD flags;
        hr = captureClient->GetBuffer(&data, &numFramesAvailable, &flags, NULL, NULL);
        if (FAILED(hr))
            break;

        std::vector<float> normalizedSamples = ConvertPCM16ToFloat(data, numFramesAvailable, waveFormat->nChannels);

        auto now = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSendTime).count();

        if (elapsedMs >= 60)
        {
            lastSendTime = now;

            json audioData = {
                {"samples", normalizedSamples},
                {"channels", waveFormat->nChannels},
                {"sampleCount", numFramesAvailable}};

            {
                std::lock_guard<std::mutex> lock(subscriptionsMutex);
                for (auto &[hwnd, msg] : audioSubscriptions)
                {
                    SendEventToHwnd(hwnd, msg, audioData, true);
                }
            }
        }

        captureClient->ReleaseBuffer(numFramesAvailable);
    }

    audioClient->Stop();
    CoTaskMemFree(waveFormat);
    CoUninitialize();
    capturing = false;
}

void StartCaptureIfNeeded()
{
    if (!capturing)
    {
        capturing = true;
        captureThread = std::thread(CaptureAudioLoop);
        captureThread.detach();
    }
}

void AddAudioSubscription(HWND hwnd, json msg)
{
    {
        std::lock_guard<std::mutex> lock(subscriptionsMutex);
        if (!audioSubscriptions.contains(hwnd) || audioSubscriptions[hwnd] != msg)
        {
            audioSubscriptions[hwnd] = msg;
        }
    }
    StartCaptureIfNeeded();
}

void RemoveAudioSubscription(HWND hwnd)
{
    if (audioSubscriptions.contains(hwnd))
    {
        std::lock_guard<std::mutex> lock(subscriptionsMutex);
        audioSubscriptions.erase(hwnd);
    }
    if (audioSubscriptions.empty())
    {
        capturing = false;
    }
}