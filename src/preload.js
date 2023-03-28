const { ipcRenderer, contextBridge } = require("electron");

// contextBridge.exposeInMainWorld("mouse", {
//     position: wp.mousePosition,
//     pressed:  wp.leftMousePressed,
//     active: wp.inForeground
// });

// contextBridge.exposeInMainWorld("system", {
//     setTaskbar: (state) => wp.setTaskbar(state)
// });

ipcRenderer.on("path", (event, content) => {
    const script = window.document.createElement("script");
    script.src = content;
    window.document.head.appendChild(script);
});

// async function mouse() {
//     const result = await ipcRenderer.invoke("mouse");
//     return result;
// }

contextBridge.exposeInMainWorld("mouse", {
    // position, active, leftMouse, middleMousePressed
    getPosition: async () => await ipcRenderer.invoke("get-mouse", "position"),
    getActive: async () => await ipcRenderer.invoke("get-mouse", "active"),
    getLeftButtonPressed: async () => await ipcRenderer.invoke("get-mouse", "leftButtonPressed"),
    getMiddleButtonPressed: async () => await ipcRenderer.invoke("get-mouse", "middleButtonPressed"),
});

contextBridge.exposeInMainWorld("media", {
    send: {
        prevTrack: () => ipcRenderer.invoke("send-media-event", -1),
        pausePlay: () => ipcRenderer.invoke("send-media-event", 0),
        nextTrack: () => ipcRenderer.invoke("send-media-event", 1)
    },
    // title, artist, secondsElapsed, secondsTotal, percentElapsed, isPlaying
    getTitle: async () => await ipcRenderer.invoke("get-media-event", "title"),
    getArtist: async () => await ipcRenderer.invoke("get-media-event", "artist"),
    getSecondsElapsed: async () => await ipcRenderer.invoke("get-media-event", "secondsElapsed"),
    getSecondsTotal: async () => await ipcRenderer.invoke("get-media-event", "secondsTotal"),
    getPercentElapsed: async () => await ipcRenderer.invoke("get-media-event", "percentElapsed"),
    isPlaying: async () => await ipcRenderer.invoke("get-media-event", "isPlaying")
});

contextBridge.exposeInMainWorld("system", {
    // themeDark, themeHighContrast, themeInverted
    isThemeDark: async () => await ipcRenderer.invoke("get-system", "themeDark"),
    isThemeLight: async () => !(await ipcRenderer.invoke("get-system", "themeDark")),
    isThemeHighContrast: async () => await ipcRenderer.invoke("get-system", "themeHighContrast"),
    isThemeInverted: async () => await ipcRenderer.invoke("get-system", "themeInverted"),
    getTheme: async () => {
        if (await ipcRenderer.invoke("get-system", "themeDark")) return "dark";
        else if (await ipcRenderer.invoke("get-system", "themeHighContrast")) return "contrast";
        else if (await ipcRenderer.invoke("get-system", "themeInverted")) return "inverted";
        else return "default";
    }
});