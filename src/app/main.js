const { app, BrowserWindow, ipcMain } = require("electron");
var main;

create();

function create() {
    app.whenReady().then(() => {
        main = new BrowserWindow({
            width: 1200,
            height: 650,
            frame: false,
            transparent: true,
            webPreferences: {
                preload: require("path").join(__dirname, "preload.js")
            }
        });
        hide();
        main.loadFile(require("path").join(__dirname, "index.html"));
        ipcMain.on("close", hide);
    });
}

function hide() {
    if (main) main.hide();
}

function show() {
    if (main) main.show();
}

function send(msg, content) {
    if (main) main.webContents.send(msg, content);
}

function dispatch(content) {
    main.webContents.executeJavaScript(content);
}

module.exports = { hide, show, send, dispatch };