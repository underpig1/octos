const { app, BrowserWindow, ipcMain } = require("electron");

app.whenReady().then(() => {
    const win = new BrowserWindow({
        width: 1200,
        height: 650,
        frame: false,
        transparent: true,
        webPreferences: {
            preload: require("path").join(__dirname, "preload.js")
        }
    });

    win.loadFile("index.html");

    ipcMain.on("close", () => win.hide());
});