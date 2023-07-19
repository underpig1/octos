const octos = (() => {
    class MediaController {
        async pausePlay() {
            window.media.send.pausePlay();
        }

        async prevTrack() {
            window.media.send.prevTrack();
        }

        async nextTrack() {
            window.media.send.nextTrack();
        }

        async getTrack() {
            return {
                title: await window.media.getTitle(),
                artist: await window.media.getArtist(),
                secondsElapsed: await window.media.getSecondsElapsed(),
                secondsTotal: await window.media.getSecondsTotal(),
                playing: await window.media.isPlaying()
            }
        }

        async getTrackTitle() {
            return window.media.getTitle();
        }

        async getTrackArtist() {
            return window.media.getArtist();
        }

        async getTrackSecondsElapsed() {
            return window.media.getSecondsElapsed();
        }

        async getTrackSecondsTotal() {
            return window.media.getSecondsTotal();
        }

        async getTrackPercentElapsed() {
            return window.media.getPercentElapsed();
        }

        async isPlaying() {
            return window.media.isPlaying();
        }

        async send(type = "pause-play") {
            if (type == "prev-track") return window.media.send.prevTrack();
            else if (type == "pause-play") return window.media.send.pausePlay();
            else if (type == "next-track") return window.media.send.nextTrack();
            else return null;
        }

        async request(type = "title") {
            if (type == "title") return window.media.getTitle();
            else if (type == "artist") return window.media.getArtist();
            else if (type == "seconds-elapsed") return window.media.getSecondsElapsed();
            else if (type == "seconds-total") return window.media.getSecondsTotal();
            else if (type == "amount-elapsed") return window.media.getPercentElapsed();
            else if (type == "state") return window.media.isPlaying();
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
        constructor(options) {
            if (options.type == "directory") this.type = "directory";
            else this.type = "file";
            if (options.extensions) this.extensions = options.extensions;
            else this.extensions = [];
            this.events = [];
        }

        request() {
            window.storage.requestFile(this.extensions).then((filepath) => {
                for (var e of this.events) {
                    if (e.event == "finish") e.listener({ filepath });
                }
            }).catch((err) => {
                for (var e of this.events) {
                    if (e.event == "error") e.listener(err);
                }
            });
            return this;
        }

        on(event, listener) {
            this.events.push({ event, listener });
            return this;
        }
    }

    class UserPreferences {
        async read(id = "") {
            return window.prefs.get(id);
        }

        async write(id = "", value = "") {
            return window.prefs.set(id, value)
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
        // cpus, arch, machine, os, mem, freemem, uptime
        getTheme: async () => window.system.getTheme(),
        toggleDevTools: async () => window.dev.toggleDevTools(),
        getCpus: async () => window.system.request("cpus"),
        getArch: async () => window.system.request("arch"),
        getMachine: async () => window.system.request("machine"),
        getOS: async () => window.system.request("os"),
        getTotalMem: async () => window.system.request("mem"),
        getFreeMem: async () => window.system.request("freemem"),
        getUptime: async () => window.system.request("uptime"),
    }

    exports = { MediaController, Storage, FileDialog, UserPreferences, system, FileSystem }
    return exports;
})();