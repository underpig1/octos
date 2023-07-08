const octos = (() => {
    class MediaController {
        async send(type = "pause-play") {
            if (type == "prev-track") return window.media.send.prevTrack();
            else if (type == "pause-play") return window.media.send.pausePlay();
            else if (type == "next-track") return window.media.send.nextTrack();
            else return null;
        }

        async retrieve(type = "title") {
            if (type == "title") return window.media.getTitle();
            else if (type == "artist") return window.media.getArtist();
            else if (type == "seconds-elapsed") return window.media.getSecondsElapsed();
            else if (type == "seconds-total") return window.media.getSecondsTotal();
            else if (type == "amount-elapsed") return window.media.getPercentElapsed();
        }

        async playState() {
            return window.media.isPlaying();
        }

        on(event, listener) {
            if (["track", "playbackstatus", "playbacktime"].includes(event)) document.addEventListener(event, (e) => listener(e.detail));
        }
    }

    class Storage {
        async read(id = "") {
            return window.storage.getStorage(id);
        }

        async write(id = "", content = "") {
            return window.storage.setStorage(id, content);
        }
    }

    class FileDialog {
        async request(extensions = []) {
            return window.storage.requestFile(extensions);
        }
    }

    class UserPreferences {
        async read(field = "") {
            return window.prefs.get(field);
        }

        async write(field = "", value = "") {
            return window.prefs.set(field, value)
        }

        on(event, listener) {
            if (event == "change") document.addEventListener("prefschange", (e) => listener(e.detail));
        }
    }

    class FileSystem {
        async readdir(path = "") {
            return window.files.readdir(path);
        }

        async open(path = "") {
            return window.files.open(path);
        }

        async extractIcon(path = "") {
            return window.files.icon(path);
        }

        async pathExists(path = "") {
            return window.files.exists(path);
        }

        async isDirectory(path = "") {
            return window.files.isDir(path);
        }
    }

    const system = {
        getTheme: async () => window.system.getTheme(),
        toggleDevTools: async () => window.dev.toggleDevTools()
    }

    exports = { MediaController, Storage, FileDialog, UserPreferences, system, FileSystem }
    return exports;
})();