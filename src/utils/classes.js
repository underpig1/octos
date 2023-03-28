class MediaController {
    pausePlay() {
        window.media.send.pausePlay();
    }

    skipToNextTrack() {
        window.media.send.nextTrack();
    }

    skipToPrevTrack() {
        window.media.send.prevTrack();
    }
}