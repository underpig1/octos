const { app, BrowserWindow, ipcMain, screen, Menu, MenuItem, Tray, dialog } = require("electron");
const wp = require("../wallpaper");
const path = require("path");
const { getPrefs, selectMod, getSelectedConfig, addMod, getSelectedEntry, restorePrefsDefaults, filterFolders } = require("./utils/store.js");
const { modifier, keyCode } = require("./utils/ascii.js");
const { send } = require("process");
var win, tray, cmenu, settings;
const lastmouse = { x: null, y: null, pressed: false, active: false };
var lastkeystate = [];
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
    win.webContents.openDevTools();

    // win.setAlwaysOnTop(true, "pop-up-menu", -1); // for mac
}

function loadHTML() {
    var entry = getSelectedEntry();
    if (entry.isUrl) win.loadURL(entry.path);
    else win.loadFile(entry.path);
    wp.attach(win);

    win.webContents.on("dom-ready", () => {
        win.webContents.insertCSS("body { user-select: none; }");
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
        { label: "Add mod", type: "normal", click: load },
        { id: "mods", label: "Installed mods", type: "submenu", submenu: Menu.buildFromTemplate(modItems) },
        { label: "Settings", type: "normal", click: () => settings ? settings.show() : null },
        { label: "Open mod folder", type: "normal", click: () => require("child_process").exec('start "" "%AppData%\\octos\\mods"') },
        { label: "Restore defaults", type: "normal", click: restore },
        { type: "separator" },
        { id: "visibility", label: "Toggle visibility", type: "checkbox", checked: true, click: toggle },
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

function updateModList() {
    filterFolders();
    var names = getPrefs().mods.map((x) => x = {
        label: x.name,
        type: "checkbox",
        checked: getPrefs().selected == x.name
    });
    for (const name of names) name.click = () => setModByName(name.label);
    createTrayMenu(names);
}

function restore() {
    restorePrefsDefaults();
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
    console.log(wp.playbackStatus());
    if (options.events.mouse) {
        var [x, y] = wp.mousePosition();
        var active = isActive();
        var pressed = [wp.leftMousePressed(), wp.middleMousePressed()];
        if (lastmouse.x != x || lastmouse.y != y) win.webContents.sendInputEvent({ type: "mouseMove", x: x, y: y });
        if (active/* || !options.events.requireFocus*/) {
            if (pressed[0] && !lastmouse.pressed[0]) {
                win.webContents.sendInputEvent({ type: "mouseDown", x: x, y: y, button: "left", clickCount: 1 });
            }
            if (lastmouse.pressed[0] && !pressed[0]) win.webContents.sendInputEvent({ type: "mouseUp", x: x, y: y, button: "left", clickCount: 1 });

            if (pressed[1] && !lastmouse.pressed[1]) win.webContents.sendInputEvent({ type: "mouseDown", x: x, y: y, button: "middle", clickCount: 1 });
            if (lastmouse.pressed[1] && !pressed[1]) win.webContents.sendInputEvent({ type: "mouseUp", x: x, y: y, button: "middle", clickCount: 1 });
        }
        lastmouse.x = x; lastmouse.y = y; lastmouse.active = JSON.stringify(active); lastmouse.pressed = pressed;
    }
    if (options.events.keyboard) {
        var keystate = wp.keyboard();
        var modifiers = [];
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
            if (!lastkeystate.includes(key)) {
                var k = key == "Backspace" ? key : keyCode(key, shift);
                if (k) {
                    win.webContents.sendInputEvent({ type: "keyDown", keyCode: k });
                    win.webContents.sendInputEvent({ type: "char", keyCode: k });
                }
            }
        }
        for (const key of lastkeystate) {
            if (!keysPressed.includes(key)) {
                var k = keyCode(key, shift);
                if (k) win.webContents.sendInputEvent({ type: "keyUp", keyCode: k });
            }
        }

        //     var keyCode = String.fromCharCode(i);
        //     if (keystate[i]) {
        //         if (!lastkeystate[i]) {
        //             win.webContents.sendInputEvent({ type: "keyDown", keyCode });
        //             win.webContents.sendInputEvent({ type: "char", keyCode });
        //         }
        //     }
        //     else if (lastkeystate[i]) win.webContents.sendInputEvent({ type: "keyUp", keyCode });
        
        lastkeystate = JSON.parse(JSON.stringify(keysPressed));
    }
}

function retrieveOptions() {
    return getSelectedConfig().options;
}

app.whenReady().then(() => {
    init();

    if (options.events.mouse || options.events.keyboard) setInterval(handleEvents, 1);

    win.webContents.send("path", path.join(__dirname, "renderer.js"));

    ipcMain.on("close", (e) => {
        const webContents = e.sender;
        const win = BrowserWindow.fromWebContents(webContents);
        win.hide();
    });
    ipcMain.handle("send-media-event", (e, state) => {
        if (isActive()) wp.sendMediaEvent(state);
    });
    ipcMain.handle("get-media-event", (e, type) => {
        // name, artist, timeline, status
        // paused, changing, stopped, playing, closed
        if (type == "title") return wp.trackTitle();
        else if (type == "artist") return wp.trackArtist();
        else if (type == "secondsElapsed") return wp.trackTimeline()[0];
        else if (type == "secondsTotal") return wp.trackTimeline()[1];
        else if (type == "percentElapsed") return wp.trackTimeline()[0] / wp.trackTimeline()[1];
        else if (type == "isPlaying") return wp.playbackStatus() == "Playing";
    });
    // ipcMain.handle("mouse", async () => {
    //     var [x, y] = wp.mousePosition();
    //     var active = wp.inForeground();
    //     var data = { x, y, pressed: wp.leftMousePressed() && active, active };
    //     return data;
    // });
});