#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#include "./main.h"

HWAVEOUT hWaveOut; // Handle to the audio device
WAVEHDR waveHdr;   // Buffer for audio data

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    
}

int main()
{
    // Open audio device
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 44100;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec*wfx.nChannels*2;
    wfx.nBlockAlign = wfx.nChannels * 2;
    wfx.wBitsPerSample = 16;

    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
    waveOutPause(hWaveOut);

    // // Resume audio playback
    // result = waveOutRestart(hWaveOut);
    // if (result != MMSYSERR_NOERROR)
    // {
    //     // Handle error
    //     return 1;
    // }

    // // Close audio device
    // waveOutClose(hWaveOut);

    return 0;
}