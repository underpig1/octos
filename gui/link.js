const { ipcRenderer, contextBridge } = require("electron");

contextBridge.exposeInMainWorld("link", {
    close: () => ipcRenderer.send("close-gui"),
    minimize: () => ipcRenderer.send("minimize-gui"),
    fullscreen: () => ipcRenderer.send("fullscreen-gui"),
    setVisibility: (state) => ipcRenderer.send("set-visibility", state),
    getVisibility: async () => await ipcRenderer.invoke("get-visibility"),
    getPrefs: async () => await ipcRenderer.invoke("get-prefs"),
    selectMod: (name) => ipcRenderer.send("select-mod", name),
    removeMod: (name) => ipcRenderer.send("remove-mod", name),
    upload: () => ipcRenderer.send("upload")
})