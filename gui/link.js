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
    upload: () => ipcRenderer.send("upload"),
    request: {
        modData: async () => await ipcRenderer.invoke("request-source-mod-data"),
        addMod: async (name) => await ipcRenderer.invoke("add-source-mod", name),
        modImage: async (path) => await ipcRenderer.invoke("request-source-image", path)
    },
    downloadMod: async (name) => await ipcRenderer.invoke("download-mod", name),
    goToSource: (name) => ipcRenderer.send("go-to-mod-source", name),
    refresh: () => ipcRenderer.send("refresh"),
    userPrefs: {
        get: async (field) => await ipcRenderer.invoke("get-user-prefs", field),
        set: (field, content) => ipcRenderer.send("set-user-prefs", field, content),
        restore: () => ipcRenderer.send("restore-default-user-prefs")
    },
    newMod: async () => await ipcRenderer.invoke("new-develop-mod"),
    renameMod: async (path, name) => await ipcRenderer.invoke("rename-develop-mod", path, name),
    openMod: async () => await ipcRenderer.invoke("open-develop-mod"),
    runMod: (path) => ipcRenderer.send("run-mod", path),
    stopMod: () => ipcRenderer.send("stop-mod"),
    toggleDebug: (state) => ipcRenderer.send("toggle-debug", state),
    developConfig: {
        get: async (path) => await ipcRenderer.invoke("get-develop-config", path),
        set: (path, data) => ipcRenderer.send("set-develop-config", path, data)
    },
    setModPrefs: (name, options) => ipcRenderer.send("set-mod-prefs", name, options),
    restoreModPrefs: (name) => ipcRenderer.send("restore-default-mod-prefs", name),
    openModFolder: (dir) => ipcRenderer.send("open-mod-folder", dir),
    buildMod: (dir) => ipcRenderer.send("build-mod", dir),
    openExternalLink: (url) => ipcRenderer.send("open-external-link", url),
    openModsFolder: () => ipcRenderer.send("open-mods-folder"),
})