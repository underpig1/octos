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
winrt::event_token mediaToken{};
winrt::event_token playbackToken{};
winrt::event_token timelineToken{};
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
    return -1;
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
    return -1;
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

int GetMediaPlaybackRate(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
    {
        auto ref = p.PlaybackRate();
        if (ref)
            return ref.Value();
    }
    return -1;
}

bool GetMediaShuffleActive(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
    {
        auto ref = p.IsShuffleActive();
        if (ref)
            return ref.Value();
    }
    return false;
}

std::string GetMediaRepeatMode(GlobalSystemMediaTransportControlsSessionPlaybackInfo p)
{
    if (p)
    {
        auto ref = p.AutoRepeatMode();
        if (ref)
        {
            MediaPlaybackAutoRepeatMode repeatMode = ref.Value();
            std::string repeatModeStr =
                (repeatMode == MediaPlaybackAutoRepeatMode::None) ? "None" : (repeatMode == MediaPlaybackAutoRepeatMode::Track) ? "Track"
                                                                         : (repeatMode == MediaPlaybackAutoRepeatMode::List)    ? "List"
                                                                                                                                : "Unknown";
            return repeatModeStr;
        }
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
    return -1;
}

int GetMediaStartTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.StartTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
    return -1;
}

int GetMediaMinSeekTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.MinSeekTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
    return -1;
}

int GetMediaMaxSeekTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.MaxSeekTime();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
    return -1;
}

int GetSeekTime(GlobalSystemMediaTransportControlsSessionTimelineProperties p)
{
    if (p)
    {
        auto duration = p.Position();
        int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return seconds;
    }
    return -1;
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
    return nullptr;
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
    return nullptr;
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
    return nullptr;
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

void HandleThumbnailRequest(HWND hwnd, json msg)
{
    GetMediaPropertiesAsync([hwnd, msg](auto mediaProps)
                            {
        json data = GetMediaThumbnail(mediaProps);
        RespondToHwnd(hwnd, msg, data, true); });
}

// EVENT LISTENERS
void SubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false);
void UnsubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session);
void SubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false);
void UnsubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session);
void SubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false);
void UnsubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session);

// EVENT: SESSION
void SubscribeToSessionChange()
{
    GetSessionManagerAsync([](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           { sessionToken = sessionManager.SessionsChanged([&](auto &&, auto &&)
                                                                           {
    auto session = sessionManager.GetCurrentSession();
    if (session && session != oldSession) {
        if (mediaToken.value != 0)
        {
            UnsubscribeToMediaProperties(oldSession);
            SubscribeToMediaProperties(session, true);
        }
        if (playbackToken.value != 0)
        {
            UnsubscribeToPlaybackInfo(oldSession);
            SubscribeToPlaybackInfo(session, true);
        }
        if (timelineToken.value != 0)
        {
            UnsubscribeToTimelineProperties(oldSession);
            SubscribeToTimelineProperties(session, true);
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

// EVENT: MEDIA
void SubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance)
{
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (mediaToken.value == 0 || replaceInstance)
    {
        mediaToken = session.MediaPropertiesChanged([](auto const &sender, auto const &)
                                                    {
            auto asyncOp = sender.TryGetMediaPropertiesAsync();
            asyncOp.Completed([](auto&& asyncOp, auto status) {
                if (status == AsyncStatus::Completed) {
                    auto mediaProps = asyncOp.GetResults();

                    json data = GetAllMediaPropertiesJson(mediaProps);
                    for (auto &[hwnd, msg] : mediaSubscriptions)
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
    if (!mediaSubscriptions.contains(hwnd) || mediaSubscriptions[hwnd] != msg)
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
    if (mediaSubscriptions.contains(hwnd))
        GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                mediaSubscriptions.erase(hwnd);
                                if (mediaSubscriptions.empty())
                                    UnsubscribeToMediaProperties(session);
                            } });
}

// EVENT: PLAYBACK
void SubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session, bool replaceInstance)
{
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (playbackToken.value == 0 || replaceInstance)
    {
        wprintf(L"\n\nabout to subscribe\n\n");
        playbackToken = session.PlaybackInfoChanged([](auto const &sender, auto const &)
                                                    {
            auto playbackInfo = sender.GetPlaybackInfo();
            json data = GetAllPlaybackInfoJson(playbackInfo);
            wprintf(L"\n\ndoing subscribe change\n\n");
            for (auto &[hwnd, msg] : playbackSubscriptions)
                SendEventToHwnd(hwnd, msg, data, true); });
    }
}

void UnsubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session)
{
    if (playbackToken.value != 0)
    {
        session.PlaybackInfoChanged(playbackToken);
        playbackToken = {};
        if (mediaToken.value == 0 && playbackToken.value == 0 && timelineToken.value == 0)
            UnsubscribeToSessionChange();
    }
}

void AddPlaybackSubscription(HWND hwnd, json msg)
{
    if (!playbackSubscriptions.contains(hwnd) || playbackSubscriptions[hwnd] != msg)
        GetSessionManagerAsync([hwnd, msg](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                SubscribeToPlaybackInfo(session);
                                playbackSubscriptions[hwnd] = msg;
                            } });
}

void RemovePlaybackSubscription(HWND hwnd)
{
    if (playbackSubscriptions.contains(hwnd))
        GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                playbackSubscriptions.erase(hwnd);
                                if (playbackSubscriptions.empty())
                                    UnsubscribeToPlaybackInfo(session);
                            } });
}

// EVENT: TIMELINE
void SubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance)
{
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (timelineToken.value == 0 || replaceInstance)
    {
        timelineToken = session.TimelinePropertiesChanged([](auto const &sender, auto const &)
                                                          {
            auto timelineInfo = sender.GetTimelineProperties();
            json data = GetAllTimelinePropertiesJson(timelineInfo);
            for (auto &[hwnd, msg] : timelineSubscriptions)
                SendEventToHwnd(hwnd, msg, data, true); });
    }
}

void UnsubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session)
{
    if (timelineToken.value != 0)
    {
        session.TimelinePropertiesChanged(timelineToken);
        timelineToken = {};
        if (mediaToken.value == 0 && playbackToken.value == 0 && timelineToken.value == 0)
            UnsubscribeToSessionChange();
    }
}

void AddTimelineSubscription(HWND hwnd, json msg)
{
    if (!timelineSubscriptions.contains(hwnd) || timelineSubscriptions[hwnd] != msg)
        GetSessionManagerAsync([hwnd, msg](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                SubscribeToTimelineProperties(session);
                                timelineSubscriptions[hwnd] = msg;
                            } });
}

void RemoveTimelineSubscription(HWND hwnd)
{
    if (timelineSubscriptions.contains(hwnd))
        GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                timelineSubscriptions.erase(hwnd);
                                if (timelineSubscriptions.empty())
                                    UnsubscribeToTimelineProperties(session);
                            } });
}

// EVENT: CLEANUP
void SubscriptionCleanup(HWND hwnd)
{
    if (!g_sessionInitialized)
        return;
    RemoveMediaSubscription(hwnd);
    RemovePlaybackSubscription(hwnd);
    RemoveTimelineSubscription(hwnd);
}

// COMMANDS
void SendMediaCommand(std::string const &cmd)
{
    GetSessionManagerAsync([cmd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           {
        auto session = sessionManager.GetCurrentSession();
        if (session)
        {
            auto controls = session.GetPlaybackInfo().Controls();
            if (!controls)
            return;
            if (cmd == "play" && controls.IsPlayEnabled())
                session.TryPlayAsync();
            else if (cmd == "pause" && controls.IsPauseEnabled())
                session.TryPauseAsync();
            else if (cmd == "toggle" && controls.IsPlayPauseToggleEnabled())
                session.TryTogglePlayPauseAsync();
            else if (cmd == "next" && controls.IsNextEnabled())
                session.TrySkipNextAsync();
            else if (cmd == "previous" && controls.IsPreviousEnabled())
                session.TrySkipPreviousAsync();
            else if (cmd == "stop" && controls.IsStopEnabled())
                session.TryStopAsync();
            else if (cmd == "enable-shuffle" && controls.IsShuffleEnabled())
                session.TryChangeShuffleActiveAsync(true);
            else if (cmd == "disable-shuffle" && controls.IsShuffleEnabled())
                session.TryChangeShuffleActiveAsync(false);
            else if (cmd == "set-repeat-track" && controls.IsRepeatEnabled())
                session.TryChangeAutoRepeatModeAsync(MediaPlaybackAutoRepeatMode::Track);
            else if (cmd == "set-repeat-list" && controls.IsRepeatEnabled())
                session.TryChangeAutoRepeatModeAsync(MediaPlaybackAutoRepeatMode::List);
            else if (cmd == "set-repeat-none" && controls.IsRepeatEnabled())
                session.TryChangeAutoRepeatModeAsync(MediaPlaybackAutoRepeatMode::None);
        } });
}

void SetPlaybackPosition(int position) // in seconds
{
    GetSessionManagerAsync([position](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           {
        auto session = sessionManager.GetCurrentSession();
        if (session)
        {
            wprintf(L"\n\nposition %i", position);
            auto controls = session.GetPlaybackInfo().Controls();
            if (controls && controls.IsPlaybackPositionEnabled()) {
                int64_t ticks = static_cast<int64_t>(position * 10'000'000);
                session.TryChangePlaybackPositionAsync(ticks);
            }
        } });
}