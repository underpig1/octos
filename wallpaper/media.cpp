#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.h>
#include <string>

#include "./main.h"

#pragma comment(lib, "windowsapp")

using namespace winrt;
using namespace Windows::Media::Control;
using namespace Windows::Foundation;

GlobalSystemMediaTransportControlsSession GetSessionHandle() {
    GlobalSystemMediaTransportControlsSessionManager SessionManager(NULL);
    IAsyncOperation session_async = SessionManager.RequestAsync();
    session_async.get();
    SessionManager = session_async.GetResults();
    return SessionManager.GetCurrentSession();
}

GlobalSystemMediaTransportControlsSessionMediaProperties GetMediaProperties() {
    auto currentSession = GetSessionHandle();
    IAsyncOperation mediaAsync = currentSession.TryGetMediaPropertiesAsync();
    mediaAsync.get();
    return mediaAsync.GetResults();
}

std::string TrackTitle()
{
    auto mediaProperties = GetMediaProperties();
    std::string title_utf8 = to_string(mediaProperties.Title());
    return title_utf8;
}