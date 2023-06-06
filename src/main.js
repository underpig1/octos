const { app, BrowserWindow, ipcMain, screen, Menu, MenuItem, Tray, dialog, nativeTheme } = require("electron");
const wp = require("../wallpaper");
const path = require("path");
const { getPrefs, selectMod, getSelectedConfig, addMod, getSelectedEntry, filterFolders, updateSettings, revertSettings, setLocalStorage, getLocalStorage } = require("./utils/store.js");
const { modifier, keyCode } = require("./utils/ascii.js");
const { send } = require("process");
const { syncPlaybackInfo, asyncPlaybackInfo, sendMediaEvent } = require("./utils/winrt.js");
const { injectHTMLByNameScript, setStylesByNameScript, getAllWidgetNames } = require("./utils/widget.js");
const mainApp = require("./app/main.js");
var win, tray, cmenu, settings;
var prevMouse = { position: {} };
var prevKeyboard = [];
var prevMediaState = {};
var options = retrieveOptions();
const isActive = () => ["octos", ""].includes(wp.fgTitle());

function createWindow() {
    const bounds = screen.getPrimaryDisplay().bounds;

    win = new BrowserWindow({
        width: bounds.width,
        height: bounds.height,
        transparent: true,
        frame: false,
        resizable: false,
        x: bounds.x,
        y: bounds.y,
        webPreferences: {
            preload: path.join(__dirname, "preload.js"),
            nodeIntegration: false,
            contextIsolation: true
        }
    });
    //win.webContents.openDevTools();

    // win.setAlwaysOnTop(true, "pop-up-menu", -1); // for mac
}

function loadHTML() {
    var entry = getSelectedEntry();
    if (entry.isUrl) win.loadURL(entry.path);
    else win.loadFile(entry.path);
    wp.attach(win);

    win.webContents.on("dom-ready", () => {
        win.webContents.insertCSS("body { user-select: none; margin: 0; } widget { display: block; }");
        handleWidgets();
    });
}

function init() {
    createWindow();
    loadHTML();
    createTray();
    updateModList();
    createSettings();
}

function createTrayMenu(modItems = []) {
    cmenu = Menu.buildFromTemplate([
        // { label: "Open app", type: "normal", click: mainApp.show },
        { type: "separator" },
        { label: "Add mod", type: "normal", click: load },
        { id: "mods", label: "Installed mods", type: "submenu", submenu: Menu.buildFromTemplate(modItems) },
        { label: "Open mod folder", type: "normal", click: () => require("child_process").exec('start "" "%AppData%\\octos\\mods"') },
        { type: "separator" },
        { id: "visibility", label: "Toggle visibility", type: "checkbox", checked: true, click: toggle },
        { label: "Toggle devtools", type: "normal", click: toggleDev },
        { type: "separator" },
        { label: "Refresh", type: "normal", click: refresh },
        { label: "Exit", type: "normal", click: exit }
    ]);
    tray.setContextMenu(cmenu);
}

function createTray() {
    tray = new Tray("img/tray.png");
    tray.setToolTip("Octos");
    createTrayMenu();
}

function setModByName(name) {
    if (selectMod(name)) {
        loadHTML();
        refresh();
    }
    updateModList();
}

function toggleDev() {
    if (!win.webContents.isDevToolsOpened()) win.webContents.openDevTools({ mode: "detach" });
    else win.webContents.closeDevTools();
}

function updateModList() {
    filterFolders();
    var names = getPrefs().mods.map((x) => x = {
        label: x.name,
        type: "checkbox",
        checked: getPrefs().selected == x.name
    });
    for (const name of names) name.click = () => setModByName(name.label);
    createTrayMenu(names);
    mainApp.dispatch("updateMods();");
}

function load() {
    dialog.showOpenDialog({ filters: [{ name: "Mod", extensions: ["omod", "zip"] }], properties: ["openFile"] }).then((content) => {
        if (!content.canceled) {
            try {
                addMod(content.filePaths[0]).then(updateModList);
            }
            catch (err) {
                console.log(err);
            }
        }
    });
}

function requestFile(extensions = false) {
    dialog.showOpenDialog({ filters: (extensions ? [{ extensions: extensions }] : []), properties: ["openFile"] }).then((content) => {
        if (!content.canceled) return content.filePaths[0];
    });
}

function toggle() {
    if (cmenu.getMenuItemById("visibility").checked) win.show();
    else win.hide();
}

function refresh() {
    win.close();
    tray.destroy();
    init();
    options = retrieveOptions();
}

function exit() {
    wp.setTaskbar(true);
    app.isQuiting = true;
    app.quit();
    tray.destroy();
}

function createSettings() {
    settings = new BrowserWindow({
        width: 1100,
        height: 600,
        frame: false,
        resizable: true,
        webPreferences: {
            preload: path.join(__dirname, "settings/preload.js")
        }
    });

    settings.loadFile("src/settings/main.html");
    settings.hide();
}

function handleEvents() {
    if (options.events.mouse) {
        var position = wp.mousePosition();
        var mouse = {
            position: { x: position[0], y: position[1] },
            active: isActive(),
            leftButtonPressed: wp.leftMousePressed(),
            middleButtonPressed: wp.middleMousePressed()
        }
        if (prevMouse.position.x != mouse.position.x || prevMouse.position.y != mouse.position.y) win.webContents.sendInputEvent({ type: "mouseMove", x: mouse.position.x, y: mouse.position.y });
        if (mouse.active) {
            if (mouse.leftButtonPressed && !prevMouse.leftButtonPressed) win.webContents.sendInputEvent({ type: "mouseDown", x: mouse.position.x, y: mouse.position.y, button: "left", clickCount: 1 });
            if (prevMouse.leftButtonPressed && !mouse.leftButtonPressed) win.webContents.sendInputEvent({ type: "mouseUp", x: mouse.position.x, y: mouse.position.y, button: "left", clickCount: 1 });

            if (mouse.middleButtonPressed && !prevMouse.middleButtonPressed) win.webContents.sendInputEvent({ type: "mouseDown", x: mouse.position.x, y: mouse.position.y, button: "middle", clickCount: 1 });
            if (prevMouse.middleButtonPressed && !mouse.middleButtonPressed) win.webContents.sendInputEvent({ type: "mouseUp", x: mouse.position.x, y: mouse.position.y, button: "middle", clickCount: 1 });
        }
        prevMouse = mouse;
    }
    if (options.events.keyboard) {
        var keystate = wp.keyboard();
        var keysPressed = [];
        var shift = false;
        for (var i = 0; i <= 255; i++) {
            if (keystate[i]) {
                var m = modifier(i);
                if (m == "Shift") shift = true;
                else if (m) keysPressed.push(m);
                else keysPressed.push(i);
            }
        }
        for (const key of keysPressed) {
            if (!prevKeyboard.includes(key)) {
                var k = keyCode(key, shift);
                if (k) {
                    win.webContents.sendInputEvent({ type: "keyDown", keyCode: k });
                    win.webContents.sendInputEvent({ type: "char", keyCode: k });
                }
            }
        }
        for (const key of prevKeyboard) {
            if (!keysPressed.includes(key)) {
                var k = keyCode(key, shift);
                if (k) win.webContents.sendInputEvent({ type: "keyUp", keyCode: k });
            }
        }
        prevKeyboard = keysPressed;
    }
    if (options.events.media) {
        var info = syncPlaybackInfo();
        if (prevMediaState.title != info.title || prevMediaState.arist != info.arist) dispatchEvent("track", { title: info.title, artist: info.artist });
        if (prevMediaState.status != info.status) dispatchEvent("playbackstatus", { status: info.status });
        if (prevMediaState.secondsElapsed != info.secondsElapsed || prevMediaState.secondsTotal != info.secondsTotal) dispatchEvent("playbacktime", { secondsElapsed: info.secondsElapsed, secondsTotal: info.secondsTotal });
        prevMediaState = JSON.parse(JSON.stringify(info));
    }
}

function retrieveOptions() {
    return getSelectedConfig().options;
}

function dispatchEvent(type, options) {
    win.webContents.executeJavaScript(`document.dispatchEvent(new CustomEvent('${type}', {detail:${JSON.stringify(options)}}));`);
}

function handleWidgets() {
    var names = getAllWidgetNames();
    for (const name of names) injectScript(injectHTMLByNameScript(name)).then(() => injectScript(setStylesByNameScript(name)));
}

function injectScript(script) {
    return win.webContents.executeJavaScript(script);
}

app.whenReady().then(() => {
    init();

    if (options.events.mouse || options.events.keyboard || options.events.media) setInterval(handleEvents, 1);
    if (options.events.media) setInterval(asyncPlaybackInfo, 1000);

    win.webContents.send("path", path.join(__dirname, "renderer.js"));

    ipcMain.on("close", (e) => {
        const webContents = e.sender;
        const win = BrowserWindow.fromWebContents(webContents);
        win.hide();
    });
    ipcMain.on("select-current-module", (e, name) => setModByName(name));
    ipcMain.handle("get-mod-list", (e) => {
        return getPrefs().mods.map((x) => x = {
            name: x.name,
            folder: x.folder,
            selected: getPrefs().selected == x.name
        });
    });
    ipcMain.on("add-module", load);
    ipcMain.on("update-settings", (e, settings) => {
        updateSettings(settings);
    });
    ipcMain.on("revert-settings", (e) => revertSettings());
    ipcMain.handle("get-settings", (e) => {
        return getPrefs().settings;
    });
    ipcMain.handle("send-media-event", (e, state) => {
        if (isActive()) //wp.sendMediaEvent(state);
            sendMediaEvent(state);
    });
    ipcMain.handle("get-media-event", (e, type) => {
        // name, artist, timeline, status
        // paused, changing, stopped, playing, closed
        // title, artist, secondsElapsed, secondsTotal, percentElapsed, isPlaying
        // if (type == "title") return wp.trackTitle();
        // else if (type == "artist") return wp.trackArtist();
        // else if (type == "secondsElapsed") return wp.trackTimeline()[0];
        // else if (type == "secondsTotal") return wp.trackTimeline()[1];
        // else if (type == "percentElapsed") return wp.trackTimeline()[0] / wp.trackTimeline()[1];
        // else if (type == "isPlaying") return wp.playbackStatus() == "Playing";
        if (type == "title") return prevMediaState.title;
        else if (type == "artist") return prevMediaState.artist;
        else if (type == "secondsElapsed") return prevMediaState.secondsElapsed;
        else if (type == "secondsTotal") return prevMediaState.secondsTotal;
        else if (type == "percentElapsed") return prevMediaState.secondsElapsed/prevMediaState.secondsTotal;
        else if (type == "isPlaying") return prevMediaState.status == "playing";
    });
    ipcMain.handle("get-mouse", (e, type) => {
        if (type == "position") return {x: wp.mousePosition()[0], y: wp.mousePosition()[1]};
        else if (type == "active") return isActive();
        else if (type == "leftButtonPressed") return wp.leftMousePressed();
        else if (type == "middleButtonPressed") return wp.middleMousePressed();
    });
    ipcMain.handle("get-keyboard", (e, type) => {
        if (type == "keys") return prevKeyboard;
    });
    ipcMain.handle("get-system", (e, type) => {
        // themeDark, themeHighContrast, themeInverted
        if (type == "themeDark") return nativeTheme.shouldUseDarkColors;
        else if (type == "themeHighContrast") return nativeTheme.shouldUseHighContrastColors;
        else if (type == "themeInverted") return nativeTheme.shouldUseInvertedColorScheme;
    });
    ipcMain.handle("get-storage", (e, type, id = "", content = "") => {
        // getStorage, setStorage, requestFile
        if (type == "getStorage" && id) return getLocalStorage(id);
        else if (type == "setStorage" && id && content) return setLocalStorage(id, content);
        else if (type == "requestFile") return requestFile(arguments[2]);
    });
    ipcMain.handle("toggle-dev-tools", toggleDev);
});