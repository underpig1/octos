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
#include "../WebView/WebView.h"

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
                if (!g_sessionManager)
                    return;
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
        if (playbackInfo)
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

// THUMBNAIL
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
    return "";
}

void GetMediaThumbnailAsync(
    GlobalSystemMediaTransportControlsSessionMediaProperties p,
    std::function<void(std::string)> callback)
{
    if (!p)
    {
        callback("");
        return;
    }

    auto thumbnailRef = p.Thumbnail();
    if (!thumbnailRef)
    {
        callback("");
        return;
    }

    thumbnailRef.OpenReadAsync().Completed(
        [callback](auto asyncOp, auto status)
        {
            if (status != winrt::Windows::Foundation::AsyncStatus::Completed)
            {
                callback("");
                return;
            }

            auto stream = asyncOp.GetResults();
            auto size = stream.Size();

            winrt::Windows::Storage::Streams::Buffer buffer((uint32_t)size);
            stream.ReadAsync(buffer, (uint32_t)size, winrt::Windows::Storage::Streams::InputStreamOptions::None).Completed([callback, buffer](auto readOp, auto readStatus)
                                                                                                                           {
                    if (readStatus != winrt::Windows::Foundation::AsyncStatus::Completed)
                    {
                        callback("");
                        return;
                    }

                    auto dataReader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);
                    std::vector<uint8_t> bytes(buffer.Length());
                    dataReader.ReadBytes(bytes);
                    std::thread([callback, bytes = std::move(bytes)]() mutable
                    {
                        std::string base64 = Base64Encode(bytes);
                        std::string result = "data:image/*;base64," + base64;
                        callback(result);
                    }).detach(); });
        });
}

// MEDIA PROPERTIES
std::string GetMediaAlbumArtist(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.AlbumArtist());
    return "";
}

std::string GetMediaAlbumTitle(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.AlbumTitle());
    return "";
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
    return "";
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
    return json{};
}

std::string GetMediaSubtitle(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.Subtitle());
    return "";
}

std::string GetMediaTitle(GlobalSystemMediaTransportControlsSessionMediaProperties p)
{
    if (p)
        return winrt::to_string(p.Title());
    return "";
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
        auto ref = p.PlaybackType();
        if (ref)
        {
            auto playbackType = ref.Value();
            std::string playbackTypeStr =
                (playbackType == MediaPlaybackType::Music) ? "Music" : (playbackType == MediaPlaybackType::Video) ? "Video"
                                                                   : (playbackType == MediaPlaybackType::Image)   ? "Image"
                                                                                                                  : "Unknown";
            return playbackTypeStr;
        }
    }
    return "";
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
    return "";
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
    return "";
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
    return json{};
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
    return json{};
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
    return json{};
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
                            { GetMediaThumbnailAsync(mediaProps, [hwnd, msg](std::string data)
                                                     { if (!data.empty()) RespondToHwnd(hwnd, msg, data, true); }); });
}

// EVENT LISTENERS
void SendAllMediaProperties(GlobalSystemMediaTransportControlsSession session);
void SubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false);
void UnsubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session);
void SendAllPlaybackInfo(GlobalSystemMediaTransportControlsSession session, bool stopped = false);
void SubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false);
void UnsubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session);
void SendAllTimelineProperties(GlobalSystemMediaTransportControlsSession session);
void SubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance = false);
void UnsubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session);
void UnsubscribeToSessionChange();

// EVENT: SESSION
void SubscribeToSessionChange()
{
    // issue: on close/open spotify, then hitting play, subscribing to playback stops (fixed for now??)
    GetSessionManagerAsync([](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           { sessionToken = sessionManager.CurrentSessionChanged([&](auto &&, auto &&)
                                                                                 { GetSessionManagerAsync([](GlobalSystemMediaTransportControlsSessionManager SM)
                                                                                                          {
                                                    wprintf(L"\n((((((( SESSIONSCHANGED FIRED \n");
                                                    auto session = SM.GetCurrentSession();// Synchronous call
                                                    if (session)
                                                    {
                                                            wprintf(L"\n)))))))) SESSIONSCHANGED FIRED \n");
                                                            // if (mediaToken.value != 0)
                                                            //     UnsubscribeToMediaProperties(oldSession);
                                                            if (!mediaSubscriptions.empty())
                                                            {SubscribeToMediaProperties(session, true);
                                                            SendAllMediaProperties(session);
                                                        }
                                                            else{
                                                                UnsubscribeToMediaProperties(session);}
                                                                    // if (playbackToken.value != 0)
                                                                    //     UnsubscribeToPlaybackInfo(oldSession);
                                                                    if (!playbackSubscriptions.empty())
                                                                        {SubscribeToPlaybackInfo(session, true);
                                                                            SendAllPlaybackInfo(session);
                                                                        }
                                                            else {UnsubscribeToPlaybackInfo(session);}
                                                                // if (timelineToken.value != 0)
                                                                //     UnsubscribeToTimelineProperties(oldSession);
                                                                if (!timelineSubscriptions.empty())
                                                                    {SubscribeToTimelineProperties(session, true);
                                                                        SendAllTimelineProperties(session);
                                                                    }
                                                            else {UnsubscribeToTimelineProperties(session);}
                                                                oldSession = session;
                                                    }
                                                    else
                                                    {
                                                        SendAllMediaProperties(oldSession);
                                                        UnsubscribeToMediaProperties(oldSession);
                                                        UnsubscribeToPlaybackInfo(oldSession);
                                                        SendAllPlaybackInfo(oldSession, true);
                                                        UnsubscribeToTimelineProperties(oldSession);
                                                        SendAllTimelineProperties(oldSession);
                                                    } }); }); });
}

void UnsubscribeToSessionChange()
{
    GetSessionManagerAsync([](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                           {
                            if (sessionToken.value != 0) {
                            sessionManager.SessionsChanged(sessionToken);
                            sessionToken = {};} });
}

// EVENT: MEDIA
void SendAllMediaProperties(GlobalSystemMediaTransportControlsSession session)
{
    auto asyncOp = session.TryGetMediaPropertiesAsync();
    asyncOp.Completed([](auto &&asyncOp, auto status)
                      {
        if (status == AsyncStatus::Completed)
        {
            auto mediaProps = asyncOp.GetResults();
            json data = GetAllMediaPropertiesJson(mediaProps);
            for (auto &[hwnd, msg] : mediaSubscriptions)
                SendEventToHwnd(hwnd, msg, data, true);
        } });
}

void SubscribeToMediaProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance)
{
    if (!session)
        return;
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (mediaToken.value == 0 || replaceInstance)
    {
        mediaToken = session.MediaPropertiesChanged([](auto const &sender, auto const &)
                                                    {
                                                        if (!sender) return;
                                                        SendAllMediaProperties(sender); });
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
    {
        if (sessionToken.value == 0)
            SubscribeToSessionChange();
        mediaSubscriptions[hwnd] = msg;
        GetSessionManagerAsync([hwnd, msg](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                SubscribeToMediaProperties(session);
                            } });
    }
}

void RemoveMediaSubscription(HWND hwnd)
{
    if (mediaSubscriptions.contains(hwnd))
    {
        mediaSubscriptions.erase(hwnd);
        GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                if (mediaSubscriptions.empty())
                                    UnsubscribeToMediaProperties(session);
                            } });
    }
}

// EVENT: PLAYBACK
void SendAllPlaybackInfo(GlobalSystemMediaTransportControlsSession session, bool stopped)
{
    auto playbackInfo = session.GetPlaybackInfo();
    if (!playbackInfo)
        return;
    json data = GetAllPlaybackInfoJson(playbackInfo);
    wprintf(L"\n\ndoing subscribe change %hs\n\n", data.dump().c_str());
    wprintf(L"\n\ndoing subscribe change %hs\n\n", data.dump().c_str());
    if (stopped)
    {
        wprintf(L"#######################WE ARE STOPPED");
        data["playbackStatus"] = "Stopped";
    }
    for (auto &[hwnd, msg] : playbackSubscriptions)
    {
        SendEventToHwnd(hwnd, msg, data, true);
        wprintf(L"HWND: %p\n", hwnd);
        wprintf(L"MSG: %hs\n", msg.dump().c_str());
    }
}

void SubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session, bool replaceInstance)
{
    if (!session)
        return;
    wprintf(L"\nsubscribing - first go\n\n");
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (playbackToken.value == 0 || replaceInstance)
    {
        wprintf(L"\n\nabout to subscribe\n\n");
        playbackToken = session.PlaybackInfoChanged([](auto const &sender, auto const &)
                                                    {
                                                        if (!sender) return;
                                                        SendAllPlaybackInfo(sender); });
    }
}

void UnsubscribeToPlaybackInfo(GlobalSystemMediaTransportControlsSession session)
{
    if (playbackToken.value != 0)
    {
        wprintf(L"\n\n************************* WE ARE SUBSUBSCRIBING\n\n");
        session.PlaybackInfoChanged(playbackToken);
        playbackToken = {};
        if (mediaToken.value == 0 && playbackToken.value == 0 && timelineToken.value == 0)
            UnsubscribeToSessionChange();
    }
}

void AddPlaybackSubscription(HWND hwnd, json msg)
{
    if (!playbackSubscriptions.contains(hwnd) || playbackSubscriptions[hwnd] != msg)
    {
        if (sessionToken.value == 0)
            SubscribeToSessionChange();
        playbackSubscriptions[hwnd] = msg;
        GetSessionManagerAsync([hwnd, msg](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                SubscribeToPlaybackInfo(session);
                            } });
    }
}

void RemovePlaybackSubscription(HWND hwnd)
{
    if (playbackSubscriptions.contains(hwnd))
    {
        playbackSubscriptions.erase(hwnd);
        GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                if (playbackSubscriptions.empty())
                                    UnsubscribeToPlaybackInfo(session);
                            } });
    }
}

// EVENT: TIMELINE
void SendAllTimelineProperties(GlobalSystemMediaTransportControlsSession session)
{
    auto timelineInfo = session.GetTimelineProperties();
    json data = GetAllTimelinePropertiesJson(timelineInfo);
    for (auto &[hwnd, msg] : timelineSubscriptions)
        SendEventToHwnd(hwnd, msg, data, true);
}

void SubscribeToTimelineProperties(GlobalSystemMediaTransportControlsSession session, bool replaceInstance)
{
    if (!session)
        return;
    if (sessionToken.value == 0)
        SubscribeToSessionChange();
    if (timelineToken.value == 0 || replaceInstance)
    {
        timelineToken = session.TimelinePropertiesChanged([](auto const &sender, auto const &)
                                                          {
                                                        if (!sender) return;
                                                        SendAllTimelineProperties(sender); });
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
    {
        if (sessionToken.value == 0)
            SubscribeToSessionChange();
        timelineSubscriptions[hwnd] = msg;
        GetSessionManagerAsync([hwnd, msg](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                SubscribeToTimelineProperties(session);
                            } });
    }
}

void RemoveTimelineSubscription(HWND hwnd)
{
    if (timelineSubscriptions.contains(hwnd))
    {
        timelineSubscriptions.erase(hwnd);
        GetSessionManagerAsync([hwnd](GlobalSystemMediaTransportControlsSessionManager sessionManager)
                               {
                            auto session = sessionManager.GetCurrentSession();
                            if (session)
                            {
                                if (timelineSubscriptions.empty())
                                    UnsubscribeToTimelineProperties(session);
                            } });
    }
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
            auto playbackInfo = session.GetPlaybackInfo();
            if (!playbackInfo)
                return;
            auto controls = playbackInfo.Controls();
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
            auto playbackInfo = session.GetPlaybackInfo();
            if (!playbackInfo)
                return;
            auto controls = playbackInfo.Controls();
            if (controls && controls.IsPlaybackPositionEnabled()) {
                int64_t ticks = static_cast<int64_t>(position * 10'000'000);
                session.TryChangePlaybackPositionAsync(ticks);
            }
        } });
}