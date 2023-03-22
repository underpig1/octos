const control = require("@nodert-win10-21h1/windows.media.control");
const info = {};
const status = ["closed", "opened", "changing", "stopped", "playing", "paused"];
var session;

function asyncPlaybackInfo() {
    return new Promise((resolve, reject) => {
        control.GlobalSystemMediaTransportControlsSessionManager.requestAsync((error, manager) => {
            session = manager.getCurrentSession();

            if (session) {
                const timeline = session.getTimelineProperties();
                info.secondsTotal = Math.round(timeline.endTime / 1000);
                info.secondsElapsed = Math.round(timeline.position / 1000);

                const playback = session.getPlaybackInfo();
                info.status = playback.playbackStatus;
                info.status = status[playback.playbackStatus];

                session.tryGetMediaPropertiesAsync((error, media) => {
                    info.title = media.title;
                    info.artist = media.artist;

                    resolve(info);
                });
            }
            else resolve({});
        });
    })
}

function syncPlaybackInfo() {
    return info;
}

function sendMediaEvent(state = 0) {
    if (session) {
        if (state == -1) session.trySkipPreviousAsync(() => null);
        else if (state == 0) session.tryTogglePlayPauseAsync(() => null);
        else if (state == 1) session.trySkipNextAsync(() => null);
    }
}

module.exports = { syncPlaybackInfo, asyncPlaybackInfo, sendMediaEvent };