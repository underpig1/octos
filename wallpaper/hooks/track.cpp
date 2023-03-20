#include <windows.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

int main()
{
    CoInitialize(NULL);

    IMMDeviceEnumerator *enumerator;
    IMMDevice *device;
    IAudioMeterInformation *meter;
    float level;

    // Create device enumerator
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                                  CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                  (void **)&enumerator);

    if (SUCCEEDED(hr))
    {
        // Get default audio renderer
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device);

        if (SUCCEEDED(hr))
        {
            // Activate audio session manager for the device
            IAudioSessionManager2 *sessionManager;
            hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
                                  NULL, (void **)&sessionManager);

            if (SUCCEEDED(hr))
            {
                // Get audio session enumerator
                IAudioSessionEnumerator *sessionEnumerator;
                hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);

                if (SUCCEEDED(hr))
                {
                    // Get audio session control for the first session
                    IAudioSessionControl *sessionControl;
                    hr = sessionEnumerator->GetSession(0, &sessionControl);

                    if (SUCCEEDED(hr))
                    {
                        // Get audio meter information for the session
                        hr = sessionControl->QueryInterface(__uuidof(IAudioMeterInformation),
                                                            (void **)&meter);

                        if (SUCCEEDED(hr))
                        {
                            // Get peak value for the audio meter
                            hr = meter->GetPeakValue(&level);

                            if (SUCCEEDED(hr))
                            {
                                // Use the audio level
                                printf("Current audio level: %.2f\n", level);
                            }

                            meter->Release();
                        }

                        sessionControl->Release();
                    }

                    sessionEnumerator->Release();
                }

                sessionManager->Release();
            }

            device->Release();
        }

        enumerator->Release();
    }

    CoUninitialize();

    return 0;
}