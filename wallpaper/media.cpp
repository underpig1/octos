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
    sessionAsync.get();
    SessionManager = sessionAsync.GetResults();
    return SessionManager.GetCurrentSession();
}

GlobalSystemMediaTransportControlsSessionMediaProperties GetMediaProperties() {
    auto currentSession = GetSessionHandle();
    if (currentSession != NULL)
    {
        IAsyncOperation mediaAsync = currentSession.TryGetMediaPropertiesAsync();
        mediaAsync.get();
        return mediaAsync.GetResults();
    }
}

std::string TrackTitle()
{
    auto mediaProperties = GetMediaProperties();
    std::string title = to_string(mediaProperties.Title());
    return title;
}

std::string TrackArtist()
{
    auto mediaProperties = GetMediaProperties();
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
}

std::string PlaybackStatus() {
    auto currentSession = GetSessionHandle();
    if (currentSession != NULL)
    {
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
}