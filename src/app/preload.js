const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("electron", {
    close: () => ipcRenderer.send("close")
});

contextBridge.exposeInMainWorld("controls", {
    selectCurrentModule: (name) => ipcRenderer.send("select-current-module", name),
    getModList: async () => await ipcRenderer.invoke("get-mod-list"),
    addModule: () => ipcRenderer.send("add-module"),
    updateSettings: (settings) => ipcRenderer.send("update-settings", settings),
    getSettings: async () => await ipcRenderer.invoke("get-settings"),
    revertSettings: () => ipcRenderer.send("revert-settings")
});