#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.Foundation.h>
#include <string>

#include "./main.h"

#pragma comment(lib, "windowsapp")

using namespace winrt;
using namespace Windows::Media;
using namespace Windows::Media::Control;
using namespace Windows::Foundation;

GlobalSystemMediaTransportControlsSession GetSessionHandle() {
    GlobalSystemMediaTransportControlsSessionManager SessionManager(NULL);
    IAsyncOperation sessionAsync = SessionManager.RequestAsync();
    auto status = sessionAsync.wait_for(std::chrono::milliseconds{100});
    if (status != AsyncStatus::Started)
    {
        if (status == AsyncStatus::Completed) {
            SessionManager = sessionAsync.GetResults();
            if (SessionManager != NULL) return SessionManager.GetCurrentSession();
        }
    }
    return NULL;
}

GlobalSystemMediaTransportControlsSessionMediaProperties GetMediaProperties() {
    auto currentSession = GetSessionHandle();
    if (currentSession == NULL) return NULL;
    IAsyncOperation mediaAsync = currentSession.TryGetMediaPropertiesAsync();
    auto status = mediaAsync.wait_for(std::chrono::milliseconds{100});
    if (status != AsyncStatus::Started) {
        if (status == AsyncStatus::Completed) return mediaAsync.GetResults();
    }
    return NULL;
}

std::string TrackTitle()
{
    auto mediaProperties = GetMediaProperties();
    if (mediaProperties == NULL) return "";
    std::string title = to_string(mediaProperties.Title());
    return title;
}

std::string TrackArtist()
{
    auto mediaProperties = GetMediaProperties();
    if (mediaProperties == NULL) return "";
    std::string artist = to_string(mediaProperties.Artist());
    return artist;
}

int* TrackTimeline()
{
    auto currentSession = GetSessionHandle();
    if (currentSession != NULL)
    {
        auto timelineProperties = currentSession.GetTimelineProperties();
        auto startTimeInSeconds = (int)timelineProperties.StartTime().count()/10000000;
        auto positionInSeconds = (int)timelineProperties.Position().count()/10000000;
        auto endTimeInSeconds = (int)timelineProperties.EndTime().count()/10000000;

        auto elapsedTimeInSeconds = positionInSeconds - startTimeInSeconds;
        auto durationInSeconds = endTimeInSeconds - startTimeInSeconds;
        int results[2] = {elapsedTimeInSeconds, durationInSeconds};
        return results;
    }
    int defaults[2] = {0, 0};
    return defaults;
}

std::string PlaybackStatus() {
    auto currentSession = GetSessionHandle();
    if (currentSession == NULL) return "";
    auto playbackInfo = currentSession.GetPlaybackInfo();
    auto playbackStatus = playbackInfo.PlaybackStatus();
    switch (playbackStatus)
    {
    case 5:
        return "Paused";
    case 2:
        return "Changing";
    case 3:
        return "Stopped";
    case 4:
        return "Playing";
    case 0:
        return "Closed";
    case 1:
        return "Opened";
    default:
        return "";
    }
}