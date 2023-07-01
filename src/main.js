process.env.NODE_NO_WARNINGS = "1";
process.on("unhandledRejection", () => {});

const { app, BrowserWindow, ipcMain, screen, Menu, MenuItem, Tray, dialog, nativeTheme } = require("electron");
const path = require("path");
const wp = require(path.join(__dirname, "../wallpaper"));
const { getPrefs, selectMod, getSelectedConfig, addMod, getSelectedEntry, filterFolders, updateSettings, revertSettings, setLocalStorage, getLocalStorage, getModPrefs, setModPrefs, resetDefaultModPrefs, removeMod, getUserPrefs, setUserPrefs } = require("./utils/store.js");
const { modifier, keyCode } = require(path.join(__dirname, "./utils/ascii.js"));
const { syncPlaybackInfo, asyncPlaybackInfo, sendMediaEvent } = require(path.join(__dirname, "./utils/winrt.js"));
const { injectHTMLByNameScript, setStylesByNameScript, getAllWidgetNames } = require(path.join(__dirname, "./utils/widget.js"));
const { requestSourceModData, requestModImageByName, addSourceModByName, goToModSource } = require(path.join(__dirname, "./utils/request.js"))
var win, tray, cmenu, settings, gui;
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

function createGUI() {
    gui = new BrowserWindow({
        width: 825,
        height: 540,
        minWidth: 825,
        minHeight: 540,
        transparent: true,
        frame: false,
        webPreferences: {
            preload: path.join(__dirname, "../gui/link.js"),
            nodeIntegration: false,
            contextIsolation: true
        }
    });
    
    gui.loadFile(path.join(__dirname, "../gui/index.html"));
    gui.hide();

    gui.on("close", function (e) {
        e.preventDefault();
        gui.hide();
    });
}

function showGUI() {
    gui.show();
}

function hideGUI() {
    gui.hide();
}

function fullscreenGUI() {
    if (!gui.isMaximized()) gui.maximize();
    else gui.unmaximize();
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
        { label: "Open", type: "normal", click: showGUI },
        { type: "separator" },
        { label: "Add mod", type: "normal", click: load },
        { id: "mods", label: "Installed mods", type: "submenu", submenu: Menu.buildFromTemplate(modItems) },
        { label: "Open mod folder", type: "normal", click: () => require("child_process").exec('start "" "%AppData%\\octos\\mods"') },
        { type: "separator" },
        { id: "visibility", label: "Toggle visibility", type: "checkbox", checked: true, click: toggle },
        { id: "toggledev", label: "Toggle devtools", type: "checkbox", checked: false, click: toggleDev },
        { type: "separator" },
        { id: "toggleboot", label: "Opens at boot", type: "checkbox", checked: true, click: toggleBoot },
        { label: "Refresh", type: "normal", click: refresh },
        { label: "Exit", type: "normal", click: exit }
    ]);
    tray.setContextMenu(cmenu);
}

function parseArgs() {
    const yargs = require("yargs/yargs");
    const { hideBin } = require("yargs/helpers");

    yargs(hideBin(process.argv)).command("init [title] [path]", "create a new mod in the given directory", (yargs) => {
        return yargs.positional("title", {
            describe: "title of mod",
            default: "very cool mod"
        }).positional("path", {
            describe: "directory in which to create a new mod folder",
            default: process.cwd()
        });
    }, (argv) => {
        initMod(path.resolve(process.cwd(), argv.path), argv.title);
    }).command("run [path]", "run mod at the given path", (yargs) => {
        return yargs.positional("path", {
            describe: "path to the file or directory containing the target mod",
            default: process.cwd()
        });
    }, (argv) => {
        var target = path.resolve(process.cwd(), argv.path);
        runMod(target);
        if (argv.debug) {
            cmenu.getMenuItemById("toggledev").checked = true;
            win.webContents.openDevTools({ mode: "detach" });
        }
        console.log("Running mod at " + target);
    }).option("devtools", {
        alias: "d",
        type: "boolean",
        description: "run with Chrome DevTools enabled"
    }).command("build [path]", "build mod at the given path into a .omod", (yargs) => {
        return yargs.positional("path", {
            describe: "path to the file or directory containing the target mod",
            default: process.cwd()
        });
    }, (argv) => {
        buildMod(path.resolve(process.cwd(), argv.path));
    }).command("add [path]", "install mod at the given filepath", (yargs) => {
        return yargs.positional("path", {
            describe: "path to the file containing the target mod",
            default: process.cwd()
        });
    }, (argv) => {
        var dir = path.resolve(process.cwd(), argv.path);
        if (require("fs").existsSync(dir)) addMod(dir);
        else console.error("Provided directory does not exist");
    }).parse();
}

function initMod(working, title, terminate = true) {
    const fs = require("fs-extra");
    if (fs.existsSync(working)) {
        var dir = path.join(working, title);
        if (!fs.existsSync(dir)) {
            fs.mkdirSync(dir);
            fs.copySync(path.resolve(__dirname, "../example/init-mod"), dir, { overwrite: false });
            var filepath = path.join(dir, "mod.json");
            var config = require(filepath);
            config.title = title;
            fs.writeFile(filepath, JSON.stringify(config, null, 2));
            console.log("Created new mod folder at " + dir);
        }
        else console.error(`Mod with name ${title} already exists at ${dir}`);
    }
    else console.error(`Directory ${working} does not exist`);
    if (!terminate) {
        app.quit();
        process.kill(0);
    }
}

function buildMod(dir, terminate = true) {
    const archiver = require("archiver");
    const fs = require("fs");
    const archive = archiver("zip", { zlib: { level: 9 } });
    var outpath = dir + ".omod";
    const stream = fs.createWriteStream(outpath);

    console.log("Building mod...");
    stream.on("close", () => {
        console.log("Mod created at " + outpath);
        if (terminate) {
            app.quit();
            process.kill(0);
        }
    });
    archive.directory(dir, false).on("error", (err) => {
        console.error("There was an issue building your mod. See documentation for manual build.");
    }).pipe(stream);
    archive.finalize();
}

function runMod(dir) {
    if (require("fs").existsSync(dir)) {
        addMod(dir).then((name) => {
            selectMod(name);
            refresh();
            setTimeout(() => removeMod(name), 1000);
        });
    }
    else console.error("Provided directory does not exist");
}

function createTray() {
    tray = new Tray(path.join(__dirname, "../img/tray.png"));
    tray.setToolTip("Octos");
    tray.on("click", showGUI);
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
    if (cmenu.getMenuItemById("toggledev").checked) win.webContents.openDevTools({ mode: "detach" });
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
    if (gui.webContents) gui.webContents.executeJavaScript("updateMods()");
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
    return new Promise((resolve, reject) => {
        dialog.showOpenDialog({ filters: (extensions ? [{ extensions: extensions }] : []), properties: ["openFile"] }).then((content) => {
            if (!content.canceled) resolve(content.filePaths[0]);
        });
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
    process.exit(1);
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

    settings.loadFile(path.join(__dirname, "settings/main.html"));
    settings.hide();
}

function handleEvents() {
    var active = isActive();
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
                if (k && active) {
                    win.webContents.sendInputEvent({ type: "keyDown", keyCode: k });
                    win.webContents.sendInputEvent({ type: "char", keyCode: k });
                }
            }
        }
        for (const key of prevKeyboard) {
            if (!keysPressed.includes(key)) {
                var k = keyCode(key, shift);
                if (k && active) win.webContents.sendInputEvent({ type: "keyUp", keyCode: k });
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

function attachHandlers() {
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
        else if (type == "percentElapsed") return prevMediaState.secondsElapsed / prevMediaState.secondsTotal;
        else if (type == "isPlaying") return prevMediaState.status == "playing";
    });
    ipcMain.handle("get-mouse", (e, type) => {
        if (type == "position") return { x: wp.mousePosition()[0], y: wp.mousePosition()[1] };
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
    ipcMain.handle("prefs", (e, type, field = "", content = "") => {
        if (type == "get" && field) return getModPrefs(field);
        else if (type == "set" && field && content) return setModPrefs(field, content);
    });

    ipcMain.on("close-gui", hideGUI);
    ipcMain.on("minimize-gui", hideGUI);
    ipcMain.on("fullscreen-gui", fullscreenGUI);
    ipcMain.on("set-visibility", (e, state) => state ? win.show() : win.hide());
    ipcMain.handle("toggle-dev-tools", toggleDev);
    ipcMain.handle("get-visibility", win.isVisible);
    ipcMain.handle("get-prefs", getPrefs);
    ipcMain.on("select-mod", (e, name) => setModByName(name));
    ipcMain.on("remove-mod", (e, name) => {
        removeMod(name);
        updateModList();
    });
    ipcMain.on("upload", load);
    ipcMain.handle("request-source-mod-data", requestSourceModData);
    ipcMain.handle("request-source-image", (e, name) => requestModImageByName(name));
    ipcMain.handle("download-mod", (e, name) => {
        return new Promise((resolve, reject) => {
            addSourceModByName(name, (filepath) => {
                if (filepath) addMod(filepath).then(resolve).catch(reject);
                else reject();
            }).catch(reject);
        });
    });
    ipcMain.on("go-to-mod-source", (e, name) => goToModSource(name));
    ipcMain.on("refresh", refresh);
    ipcMain.on("set-user-prefs", (e, field = "", content = "") => setUserPrefs(field, content));
    ipcMain.handle("get-user-prefs", (e, field = "") => getUserPrefs(field));
    ipcMain.handle("new-develop-mod", () => {
        return new Promise((resolve, reject) => dialog.showOpenDialog(gui, "Choose a folder for your mod", { properties: ["openDirectory"] }).then((result) => {
            if (!result.canceled) {
                if (result.filePaths.length > 0) {
                    initMod(result.filePaths[0], "myMod", false);
                    resolve(result);
                };
            }
            else resolve(null);
        }));
    });
    ipcMain.handle("rename-develop-mod", (e, dir, name) => {
        if (dir && name) {
            var folder = path.join(path.dirname(dir), name);
            require("fs").renameSync(dir, folder);
            return folder;
        }
        else return null;
    });
    ipcMain.handle("open-mod", () => {
        return new Promise((resolve, reject) => dialog.showOpenDialog(gui, "Choose a mod folder to open", { properties: ["openDirectory"] }).then((result) => {
            if (!result.canceled) {
                if (result.filePaths.length > 0) resolve(result.filePaths[0]);
            }
            else resolve(null);
        }));
    });
    ipcMain.on("run-mod", (e, dir) => runMod(dir));
    ipcMain.on("stop-mod", refresh);
    ipcMain.on("toggle-debug", (e, state) => {
        if (state) win.webContents.openDevTools({ mode: "detach" });
        else win.webContents.closeDevTools();
    });
    // ipcMain.handle("init-mod", () => {
    //     return new Promise((resolve, reject) => dialog.showOpenDialog(gui, "Choose a folder for your mod", { properties: ["openDirectory"]}).then((result) => {
    //         if (!result.canceled) {
    //             if (result.filePaths.length > 0) {
    //                 initMod(result.filePaths[0], "myMod", false);
    //                 resolve(result);
    //             };
    //         }
    //         else resolve(null);
    //     }));
    // });
    // ipcMain.handle("run-mod", (dir) => runMod(dir, false));
}

function setOpenAtBoot(state) {
    app.setLoginItemSettings({
        openAtLogin: state,
        path: process.execPath
    });
}

function toggleBoot() {
    setOpenAtBoot(cmenu.getMenuItemById("toggleboot").checked);
}

parseArgs();
app.whenReady().then(() => {
    createGUI();
    init();

    if (options.events.mouse || options.events.keyboard || options.events.media) setInterval(handleEvents, 1);
    if (options.events.media) setInterval(asyncPlaybackInfo, 100);

    win.webContents.send("path", path.join(__dirname, "renderer.js"));
    attachHandlers();

    if (process.argv[1] == "--squirrel-firstrun") {
        showGUI();
        setOpenAtBoot(true);

        require("child_process").execSync(`REG ADD HKCU\\Software\\Classes\\.omod /ve /d "octos.OctosFile" /f
    REG ADD HKCU\\Software\\Classes\\octos.OctosFile /ve /d "Octos File" /f
    REG ADD HKCU\\Software\\Classes\\octos.OctosFile\\DefaultIcon /ve /d "${path.join(__dirname, "img/omod.ico")}" /f
    REG ADD HKCU\\Software\\Classes\\octos.OctosFile\\Shell\\Open\\Command /ve /d "\"${process.execPath}\" add \"%1\"" /f
    SET PATH=%PATH%;${process.execPath}`, { windowsHide: true });
    }
});