#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/base.h>
#include <shlwapi.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <unordered_set>

#include "API.h"

using namespace winrt;
using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Media;
using json = nlohmann::json;

GlobalSystemMediaTransportControlsSessionManager g_sessionManager = nullptr;
bool g_sessionInitialized = false;
bool g_initializing = false;
std::vector<std::function<void(GlobalSystemMediaTransportControlsSessionManager)>> g_initCallbacks;

winrt::event_token sessionToken;
GlobalSystemMediaTransportControlsSession oldSession = nullptr;
winrt::event_token mediaToken;
winrt::event_token playbackToken;
winrt::event_token timelineToken;
std::unordered_map<HWND, json> mediaSubscriptions;
std::unordered_map<HWND, json> playbackSubscriptions;
std::unordered_map<HWND, json> timelineSubscriptions;

// ASYNC GET SESSION PROPS/INFO
void GetSessionManagerAsync(std::function<void(GlobalSystemMediaTransportControlsSessionManager)> callback)
{
    if (g_sessionInitialized)
    {
        callback(g_sessionManager);
        return;
    }
    g_initCallbacks.push_back(callback);
    if (!g_initializing)
    {
        g_initializing = true;
        GlobalSystemMediaTransportControlsSessionManager::RequestAsync().Completed([](auto &&sender, auto &&)
                                                                                   {
            if (sender.Status() == AsyncStatus::Completed)
            {
                g_sessionManager = sender.GetResults();
                g_sessionInitialized = true;
                for (auto& cb : g_initCallbacks)
                    cb(g_sessionManager);
                g_initCallbacks.clear();
            }
            else
                g_initCallbacks.clear(); });
    }
}

void GetMediaPropertiesAsync(std::function<void(GlobalSystemMediaTransportControlsSessionMediaProperties)> callback)
{
    GetSessionManagerAsync([callback](GlobalSystemMediaTransportControlsSessionManager manager)
                           {
    auto currentSession = manager.GetCurrentSession();
    if (currentSession)
    {
        auto asyncMediaProps = currentSession.TryGetMediaPropertiesAsync();
        asyncMediaProps.Completed([callback](auto &&sender, auto &&args)
                                  {
                    if (sender.Status() == AsyncStatus::Completed)
                    {
                        auto mediaProps = sender.GetResults();
                        callback(mediaProps);
                    } });
    } });
}

void GetPlaybackInfoAsync(std::function<void(GlobalSystemMediaTransportControlsSessionPlaybackInfo)> callback)
{
    GetSessionManagerAsync([callback](GlobalSystemMediaTransportControlsSessionManager manager)
                           {
    auto currentSession = manager.GetCurrentSession();
    if (currentSession)
    {
        auto playbackInfo = currentSession.GetPlaybackInfo();
        callback(playbackInfo);
    }; });
}

void GetTimelinePropertiesAsync(std::function<void(GlobalSystemMediaTransportControlsSessionTimelineProperties)> callback)
{
    GetSessionManagerAsync([callback](GlobalSystemMediaTransportControlsSessionManager manager)
                           {
    auto currentSession = manager.GetCurrentSession();
    if (currentSession)
    {
        auto timelineProps = currentSession.GetTimelineProperties();
        callback(timelineProps);
    }; });
}

// MEDIA PROPERTIES
std::string Base64Encode(const std::vector<uint8_t> &data)
{
    DWORD size = 0;
    CryptBinaryToStringA(data.data(), (DWORD)data.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &size);
    std::string base64(size, '\0');
    CryptBinaryToStringA(data.data(), (DWORD)data.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64.data(), &size);
    return base64;
}

std::string GetMediaThumbnail(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
    {
        auto thumbnailRef = p.Thumbnail();
        if (!thumbnailRef)
            return "";

        auto stream = thumbnailRef.OpenReadAsync().get();
        auto size = stream.Size();

        IBuffer buffer = winrt::Windows::Storage::Streams::Buffer((uint32_t)size);
        stream.ReadAsync(buffer, (uint32_t)size, InputStreamOptions::None).get();

        std::vector<uint8_t> bytes(buffer.Length());
        winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer).ReadBytes(bytes);

        return "data:image/*;base64," + Base64Encode(bytes);
    }
    return nullptr;
}

std::string GetMediaAlbumArtist(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.AlbumArtist());
    return nullptr;
}

std::string GetMediaAlbumTitle(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.AlbumTitle());
    return nullptr;
}

int32_t GetMediaAlbumTrackCount(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return p.AlbumTrackCount();
}

std::string GetMediaArtist(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.Artist());
    return nullptr;
}

json GetMediaGenres(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
    {
        Collections::IVectorView<winrt::hstring> genres = p.Genres();
        json genresJson = json::array();
        for (uint32_t i = 0; i < genres.Size(); ++i)
        {
            genresJson.push_back(winrt::to_string(genres.GetAt(i)));
        }
        return genresJson;
    }
    return nullptr;
}

std::string GetMediaSubtitle(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.Subtitle());
    return nullptr;
}

std::string GetMediaTitle(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.Title());
    return nullptr;
}

int32_t GetMediaTrackNumber(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return p.TrackNumber();
}

// PLAYBACK
std::string GetMediaPlaybackType(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
    {
        auto playbackType = p.PlaybackType().Value();
        std::string playbackTypeStr =
            (playbackType == MediaPlaybackType::Music) ? "Music" : (playbackType == MediaPlaybackType::Video) ? "Video"
                                                               : (playbackType == MediaPlaybackType::Image)   ? "Image"
                                                                                                              : "Unknown";
        return playbackTypeStr;
    }
    return nullptr;
}

using PlaybackStatus = GlobalSystemMediaTransportControlsSessionPlaybackStatus;
std::string GetMediaPlaybackStatus(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
    {
        PlaybackStatus status = p.PlaybackStatus();
        std::string playbackTypeStr =
            (status == PlaybackStatus::Closed) ? "Closed" : (status == PlaybackStatus::Opened) ? "Opened"
                                                        : (status == PlaybackStatus::Changing) ? "Changing"
                                                        : (status == PlaybackStatus::Stopped)  ? "Stopped"
                                                        : (status == PlaybackStatus::Playing)  ? "Playing"
                                                        : (status == PlaybackStatus::Paused)   ? "Paused"
                                                                                               : "Unknown";
        return playbackTypeStr;
    }
    return nullptr;
}

double GetMediaPlaybackRate(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
        return p.PlaybackRate().Value();
}

bool GetMediaShuffleActive(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
        return p.IsShuffleActive().Value();
}

std::string GetMediaRepeatMode(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
    {
        MediaPlaybackAutoRepeatMode repeatMode = p.AutoRepeatMode().Value();
        std::string repeatModeStr =
            (repeatMode == MediaPlaybackAutoRepeatMode ::None) ? "None" : (repeatMode == MediaPlaybackAutoRepeatMode ::Track) ? "Track"
                                                                      : (repeatMode == MediaPlaybackAutoRepeatMode ::List)    ? "List"
                                                                                                                              : "Unknown";
        return repeatModeStr;
    }
    return nullptr;
}

// TIMELINE
int GetMediaEndTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.EndTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
}

int GetMediaStartTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.StartTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
}

int GetMediaMinSeekTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.MinSeekTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
}

int GetMediaMaxSeekTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.MaxSeekTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
}

int GetSeekTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.Position();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
}

// AGGREGATE
json GetAllMediaPropertiesJson(GlobalSystemMediaTransportControlsSessionMediaProperties p, bool includeThumbnail = false)
{
    if (p)
    {
        json details = {
            {"title", GetMediaTitle(p)},
            {"subtitle", GetMediaSubtitle(p)},
            {"albumArtist", GetMediaAlbumArtist(p)},
            {"albumTitle", GetMediaAlbumTitle(p)},
            {"albumTrackCount", GetMediaAlbumTrackCount(p)},
            {"artist", GetMediaArtist(p)},
            {"genres", GetMediaGenres(p)},
            {"trackNumber", GetMediaTrackNumber(p)},
        };
        if (includeThumbnail)
            details["thumbnail"] = GetMediaThumbnail(p);
        return details;
    }
}

json GetAllPlaybackInfoJson(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
        return {
            {"playbackStatus", GetMediaPlaybackStatus(p)},
            {"playbackRate", GetMediaPlaybackRate(p)},
            {"shuffleActive", GetMediaShuffleActive(p)},
            {"repeatMode", GetMediaRepeatMode(p)},
            {"playbackType", GetMediaPlaybackType(p)},
        };
}

json GetAllTimelinePropertiesJson(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
        return {
            {"startTime", GetMediaStartTime(p)},
            {"endTime", GetMediaEndTime(p)},
            {"position", GetSeekTime(p)},
            {"minSeekTime", GetMediaMinSeekTime(p)},
            {"maxSeekTime", GetMediaMaxSeekTime(p)},
        };
}

// DISPATCH
void HandleMediaPropertiesRequest(HWND hwnd, json msg)
{
    GetMediaPropertiesAsync([hwnd, msg](auto mediaProps)
                            {
        json data = GetAllMediaPropertiesJson(mediaProps);
        RespondToHwnd(hwnd, msg, data, true); });
}

void HandlePlaybackInfoRequest(HWND hwnd, json msg)
{
    GetPlaybackInfoAsync([hwnd, msg](auto playbackInfo)
                         {
        json data = GetAllPlaybackInfoJson(playbackInfo);
        RespondToHwnd(hwnd, msg, data, true); });
}

void HandleTimelinePropertiesRequest(HWND hwnd, json msg)
{
    GetTimelinePropertiesAsync([hwnd, msg](auto timelineProps)
                               {
        json data = GetAllTimelinePropertiesJson(timelineProps);
        RespondToHwnd(hwnd, msg, data, true); });
}

// EVENT LISTENERS
void SubscribeToSessionChange()
{
    GetSessionManagerAsync([](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           { sessionToken = sessionManager.SessionsChanged([&](auto &&, auto &&)
                                                                           {
    auto session = sessionManager.GetCurrentSession();
    if (session) {
        if (mediaToken.value != 0)
        {
            UnsubscribeToMediaProperties(oldSession);
            SubscribeToMediaProperties(session, true);
        }
        if (playbackToken.value != 0)
        {
            UnsubscribeToMediaProperties(oldSession);
            SubscribeToMediaProperties(session, true);
        }
        if (timelineToken.value != 0)
        {
            UnsubscribeToMediaProperties(oldSession);
            SubscribeToMediaProperties(session, true);
        }
        oldSession = session;
    } }); });
}

void UnsubscribeToSessionChange()
{
    GetSessionManagerAsync([](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           {
                            sessionManager.SessionsChanged(sessionToken);
                            sessionToken = {}; });
}

void SubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false)
{
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (mediaToken.value == 0 || replaceInstance)
    {
        mediaToken = session.MediaPropertiesChanged([](auto const &sender, auto const &)
                                                    {
            auto asyncOp = sender.TryGetMediaPropertiesAsync();
            asyncOp.Completed([](auto&& asyncOp, auto status) {
                if (status == winrt::AsyncStatus::Completed) {
                    auto mediaProps = asyncOp.GetResults();

                    json data = GetAllMediaPropertiesJson(mediaProps);
                    for (auto &[hwnd, msg] : mediaSubscription)
                        SendEventToHwnd(hwnd, msg, data, true);
                }
            }); });
    }
}

void UnsubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session)
{
    if (mediaToken.value != 0)
    {
        session.MediaPropertiesChanged(mediaToken);
        mediaToken = {};
        if (mediaToken.value == 0 && playbackToken.value == 0 && timelineToken.value == 0)
            UnsubscribeToSessionChange();
    }
}

void AddMediaSubscription(HWND hwnd, json msg)
{
    GetSessionManagerAsync([hwnd, msg](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                SubscribeToMediaProperties(session);
                                mediaSubscriptions[hwnd] = msg;
                            } });
}

void RemoveMediaSubscription(HWND hwnd)
{
    GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                UnsubscribeToMediaProperties(session);
                                mediaSubscriptions.erase(hwnd);
                            } });
}