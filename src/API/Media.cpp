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

void MTACallback(std::function<void()> func)
{
    std::thread([func = std::move(func)]
                {
        winrt::init_apartment(apartment_type::multi_threaded);
        func(); })
        .detach();
}

void InitializeMediaSession()
{
    winrt::init_apartment();
    g_sessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    g_sessionInitialized = true;
}

GlobalSystemMediaTransportControlsSession GetCurrentMediaSession()
{
    if (!g_sessionInitialized)
        InitializeMediaSession();
    auto session = g_sessionManager.GetCurrentSession();
    return session;
}

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

// mediaProperties.AlbumArtist();
// mediaProperties.AlbumTitle();
// mediaProperties.AlbumTrackCount();
// mediaProperties.Artist();
// mediaProperties.Genres();
// mediaProperties.PlaybackType();
// mediaProperties.Subtitle();
// mediaProperties.Title();
// mediaProperties.TrackNumber();

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
    return nullptr;
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

json GetMediaPropertyJson(
    const std::string &key,
    GlobalSystemMediaTransportControlsSessionMediaProperties mediaProps)
{
    if (key == "title")
        return GetMediaTitle(mediaProps);
    else if (key == "subtitle")
        return GetMediaSubtitle(mediaProps);
    else if (key == "albumArtist")
        return GetMediaAlbumArtist(mediaProps);
    else if (key == "albumTitle")
        return GetMediaAlbumTitle(mediaProps);
    else if (key == "albumTrackCount")
        return GetMediaAlbumTrackCount(mediaProps);
    else if (key == "artist")
        return GetMediaArtist(mediaProps);
    else if (key == "genres")
        return GetMediaGenres(mediaProps);
    else if (key == "trackNumber")
        return GetMediaTrackNumber(mediaProps);
    else if (key == "thumbnail")
        return GetMediaThumbnail(mediaProps);
    return nullptr;
}

json GetPlaybackJson(const std::string &key, GlobalSystemMediaTransportControlsSessionPlaybackInfo playbackInfo)
{

    if (key == "playbackStatus")
        return GetMediaPlaybackStatus(playbackInfo);
    else if (key == "playbackRate")
        return GetMediaPlaybackRate(playbackInfo);
    else if (key == "shuffleActive")
        return GetMediaShuffleActive(playbackInfo);
    else if (key == "playbackType")
        return GetMediaPlaybackType(playbackInfo);
    else if (key == "repeatMode")
        return GetMediaRepeatMode(playbackInfo);
    return nullptr;
}

json GetTimelinePropertyJson(const std::string &key, GlobalSystemMediaTransportControlsSessionTimelineProperties timelineProps)
{

    if (key == "startTime")
        return GetMediaStartTime(timelineProps);
    else if (key == "endTime")
        return GetMediaEndTime(timelineProps);
    else if (key == "position")
        return GetSeekTime(timelineProps);
    else if (key == "minSeekTime")
        return GetMediaMinSeekTime(timelineProps);
    else if (key == "maxSeekTime")
        return GetMediaMaxSeekTime(timelineProps);
    return nullptr;
}

void GetMediaPropertiesAsync(
    GlobalSystemMediaTransportControlsSession const &session,
    std::function<void(GlobalSystemMediaTransportControlsSessionMediaProperties)> callback)
{
    if (!session)
        return;
    auto asyncOp = session.TryGetMediaPropertiesAsync();
    asyncOp.Completed([callback](auto const &sender, auto const &)
                      {
        try
        {
            auto mediaProps = sender.GetResults();
            callback(mediaProps);
        }
        catch (...)
        {} });
}

GlobalSystemMediaTransportControlsSessionPlaybackInfo GetPlaybackInfo(GlobalSystemMediaTransportControlsSession const &session)
{
    auto playbackInfo = session.GetPlaybackInfo();
    return playbackInfo;
}

GlobalSystemMediaTransportControlsSessionTimelineProperties GetTimelineProperties(GlobalSystemMediaTransportControlsSession const &session)
{
    auto timelineProps = session.GetTimelineProperties();
    return timelineProps;
}

void DispatchMediaProperty(HWND hwnd, json msg, std::string key)
{
    auto session = GetCurrentMediaSession();
    GetMediaPropertiesAsync(session, [hwnd, msg, key](GlobalSystemMediaTransportControlsSessionMediaProperties p)
                            {
                                json data = GetMediaPropertyJson(key, p);
                                RespondToHwnd(hwnd, msg, data); });
}

void DispatchPlayback(HWND hwnd, json msg, std::string key)
{
    auto session = GetCurrentMediaSession();
    auto p = GetPlaybackInfo(session);
    json data = GetPlaybackJson(key, p);
    RespondToHwnd(hwnd, msg, data);
}

void DispatchTimelineProperty(HWND hwnd, json msg, std::string key)
{
    wprintf(L"on timeline\n");
    auto session = GetCurrentMediaSession();
    wprintf(L"on timeline1\n");
    auto p = GetTimelineProperties(session);
    wprintf(L"on timeline2\n");
    json data = GetTimelinePropertyJson(key, p);
    wprintf(L"on timeline3\n");
    RespondToHwnd(hwnd, msg, data);
    wprintf(L"on timeline4\n");
}

bool RouteArbitraryMediaRequest(HWND hwnd, json msg, std::string key)
{
    static const std::unordered_map<std::string, std::unordered_set<std::string>> keyMap = {
        {"media", {"title", "subtitle", "albumArtist", "albumTitle", "albumTrackCount", "artist", "genres", "trackNumber", "thumbnail"}},
        {"playback", {"playbackStatus", "playbackRate", "shuffleActive", "playbackType", "repeatMode"}},
        {"timeline", {"startTime", "endTime", "position", "minSeekTime", "maxSeekTime"}}};

    size_t dashPos = key.find('-');
    if (dashPos == std::string::npos)
        return false;

    std::string prefix = key.substr(0, dashPos);
    std::string actualKey = key.substr(dashPos + 1);

    auto it = keyMap.find(prefix);
    if (it != keyMap.end() && it->second.count(actualKey))
    {
        if (prefix == "media")
        {
            DispatchMediaProperty(hwnd, msg, actualKey);
        }
        else if (prefix == "playback")
        {
            DispatchPlayback(hwnd, msg, actualKey);
        }
        else if (prefix == "timeline")
        {
            DispatchTimelineProperty(hwnd, msg, actualKey);
        }
        return true;
    }

    return false;
}

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
            {"thumbnail", GetMediaThumbnail(p)},
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

void SubscribeToMediaSessionChange()
{
    g_sessionManager.CurrentSessionChanged([](auto const &sender, auto const &)
                                           {
            auto session = sender.GetCurrentSession();
            SubscribeToMediaPropertiesChange(session);
            SubscribeToPlaybackInfoChange(session);
            SubscribeToTimelinePropertiesChange(session); });
}

void SubscribeToMediaPropertiesChange(GlobalSystemMediaTransportControlsSession session)
{
    if (session)
    {
        session.MediaPropertiesChanged([](auto const &sender, auto const &)
                                       {  });
    }
}

void SubscribeToPlaybackInfoChange(GlobalSystemMediaTransportControlsSession session)
{
    if (session)
    {
        session.PlaybackInfoChanged([](auto const &sender, auto const &)
                                    {  });
    }
}

void SubscribeToTimelinePropertiesChange(GlobalSystemMediaTransportControlsSession session)
{
    if (session)
    {
        session.TimelinePropertiesChanged([](auto const &sender, auto const &)
                                          {  });
    }
}