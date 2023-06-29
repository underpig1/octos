const electron = require("electron");
const path = require("path");
const fs = require("fs-extra");
const unzipper = require("unzipper");

const defaults = require("../../default-mod/defaults.json");

var prefs;
const userDataPath = (electron.app || electron.remote.app).getPath("userData");
const prefPath = path.join(userDataPath, "preferences.json");
const modsPath = path.join(userDataPath, "mods");
const defaultsPath = path.join(__dirname, "../..", "default-mod");

initStore();
filterFolders();

// EXPORTS

function initStore() {
    prefs = fillObject(defaults, handleFile(prefPath, defaults));
    loadDefaultMods();
}

function selectMod(name) {
    var data = getModData(name);
    if (data) {
        if (checkModFolderExists(data.folder)) {
            prefs.selected = name;
            writePrefs();
            return true;
        }
    }
    return false;
}

function getSelectedModData() {
    checkSelectedExists();
    return getModData(prefs.selected);
}

function getSelectedConfig() {
    checkSelectedExists();
    try {
        return readModConfig(prefs.selected);
    }
    catch (err) {
        return getConfigFromFolderPath(path.join(defaultsPath, defaults.selected));
    }
}

function removeMod(name) {
    var data = getModData(name);
    var index = prefs.mods.indexOf(data);
    prefs.mods.splice(index, 1);
    writePrefs();
}

function addMod(modPath) {
    return new Promise((resolve, reject) => {
        handleModFolderCopy(modPath).then((folder) => {
            if (folder) {
                var config = getConfigFromFolderPath(resolveModFolder(folder));
                var name = config.name;
                if (name == "Custom mod" || name == "" || !name) name = folder;
                addModMetadata(name, config, folder);
                addModData(name, folder);
                writePrefs();
                resolve(name);
            }
            else resolve();
        }).catch(reject);
    });
}

function getSelectedEntry() {
    checkSelectedExists();
    try {
        var entryPath = getSelectedConfig().entry;
        try {
            var url = new URL(entryPath);
            return { path: entryPath, isUrl: true };
        }
        catch (err) {
            var folder = resolveModFolder(getModFolder(prefs.selected));
            return { path: path.join(folder, entryPath), isUrl: false };
        }
    }
    catch (err) {
        return { path: path.join(__dirname, "../..", defaultsPath, defaults.selected, "index.html"), isUrl: false }
    }
}

function restorePrefsDefaults() {
    prefs = defaults;
    writePrefs();
}

function loadDefaultMods() {
    var dirs = fs.readdirSync(defaultsPath, { withFileTypes: true }).filter((dirent) => dirent.isDirectory()).map((dir) => dir.name);
    for (const dir of dirs) addMod(path.resolve(path.join(defaultsPath, dir)));
}

function getPrefs() {
    return prefs;
}

function filterFolders() {
    for (const mod of prefs.mods) {
        if (!checkModFolderExists(mod.folder)) {
            removeMod(mod.name);
        }
    }
}

function updateSettings(settings) {
    prefs.settings = settings;
    writePrefs();
}

function revertSettings() {
    prefs.settings = defaults;
    writePrefs();
}

function setLocalStorage(id, data) {
    if (prefs.local.hasOwnProperty(prefs.selected)) {
        prefs.local[prefs.selected][id] = data;
    }
    else {
        prefs.local[prefs.selected] = {};
        prefs.local[prefs.selected][id] = data;
    }
    writePrefs();
}

function getLocalStorage(id) {
    if (prefs.local.hasOwnProperty(prefs.selected)) {
        if (prefs.local[prefs.selected].hasOwnProperty(id)) {
            return prefs.local[prefs.selected][id];
        }
    }
    else {
        prefs.local[prefs.selected] = {};
        prefs.local[prefs.selected][id] = {};
        writePrefs();
        return null;
    }
}

function getModPrefs(field) {
    if (prefs.prefs[prefs.selected]) {
        var modPrefs = prefs.prefs[prefs.selected].local;
        if (modPrefs) {
            if (modPrefs[field]) return modPrefs[field].value;
        }
    }
    return null;
}

function setModPrefs(field, content) {
    if (prefs.prefs[prefs.selected]) {
        var modPrefs = prefs.prefs[prefs.selected].local;
        if (modPrefs) {
            if (modPrefs[field]) {
                modPrefs[field].value = content;
                writePrefs();
            }
        }
    }
}

function resetDefaultModPrefs() {
    if (prefs.prefs[prefs.selected]) {
        prefs.prefs[prefs.selected].local = prefs.prefs[prefs.selected].defaults;
        writePrefs();
    }
}

// UTILITIES

function copyModFolder(target) {
    return new Promise((resolve, reject) => {
        var folderName = path.basename(target);
        fs.copy(target, resolveModFolder(folderName)).then(() => resolve(folderName)).catch(() => {});
    });
}

function unzipAndCopyModFolder(target) {
    return new Promise((resolve, reject) => {
        var folderName = path.basename(target).replace(/\.omod$/, "").replace(/\.zip$/, "");
        var destination = resolveModFolder(folderName);
        if (!fs.existsSync(destination)) fs.mkdirSync(destination);
        else fs.emptyDirSync(destination);
        fs.createReadStream(target).on("entry", entry => entry.autodrain()).pipe(unzipper.Parse()).on("entry", (entry) => {
            entry.pipe(fs.createWriteStream(path.join(destination, entry.path)));
        }).on("finish", () => {
            resolve(folderName);
        }).on("error", () => {});
    });
}

function handleModFolderCopy(target) {
    if (target.endsWith(".omod") || target.endsWith(".zip")) return unzipAndCopyModFolder(target);
    else if (fs.lstatSync(target).isDirectory()) return copyModFolder(target);
    else return Promise.reject();
}

function checkSelectedExists() {
    if (!checkModDataExists(prefs.selected)) {
        loadDefaultMods();
        try {
            if (!checkModDataExists(prefs.selected)) prefs.selected = prefs.mods[0].name;
        }
        catch {}
    }
    if (!getModData(prefs.selected)) {
        try {
            prefs.selected = prefs.mods[0].name;
            writePrefs();
        }
        catch {}
    }
}

function addModData(name, folder) {
    var newData = { name, folder };
    var data = getModData(name);
    if (data) {
        var index = prefs.mods.indexOf(data);
        prefs.mods[index] = newData;
    }
    else prefs.mods.push(newData);
}

function addModMetadata(name, config, folder) {
    var modPrefs = config.prefs;
    if (modPrefs) {
        if (!prefs.prefs) prefs.prefs = {};
        if (!prefs.prefs[name]) prefs.prefs[name] = { defaults: modPrefs, local: modPrefs };
        else prefs.prefs[name].defaults = modPrefs;
    }
    if (!prefs.configs) prefs.configs = {};
    prefs.configs[name] = config;
    var relativeImagePath = config.image;
    if (relativeImagePath) {
        if (!prefs.images) prefs.images = {};
        try {
            new URL(relativeImagePath);
        }
        catch (err) {
            relativeImagePath = path.resolve(resolveModFolder(folder), config.image);
        }
        prefs.images[name] = relativeImagePath;
    }
    return name;
}

function getModData(name) {
    return prefs.mods.find((value) => value.name == name);
}

function getModFolder(name) {
    return getModData(name).folder;
}

function checkModDataExists(name) {
    return getModData(name) != undefined;
}

function resolveModFolder(folder) {
    return path.join(modsPath, folder);
}

function getConfigPathFromFolderPath(folder) {
    return path.join(folder, "mod.json");
}

function getConfigFromFolderPath(path) {
    var configPath = getConfigPathFromFolderPath(path);
    return fillObject(defaults.config, handleFile(configPath, defaults.config));
}

function readModConfig(name) {
    var folder = getModFolder(name);
    var path = resolveModFolder(folder);
    return getConfigFromFolderPath(path);
}

function checkModFolderExists(folder) {
    if (folder) return fs.existsSync(resolveModFolder(folder));
    return false;
}

function handleFile(path, defaults = "") {
    try {
        return JSON.parse(fs.readFileSync(path));
    }
    catch {
        fs.writeFileSync(path, JSON.stringify(defaults));
        return defaults;
    }
}

function writePrefs() {
    try {
        fs.writeFileSync(prefPath, JSON.stringify(prefs));
    }
    catch {
        restorePrefsDefaults();
    }
}

function fillObject(defer, overwrite) {
    return { ...defer, ...overwrite };
}

// EXPORT

module.exports = { getPrefs, initStore, selectMod, getSelectedModData, getSelectedConfig, removeMod, addMod, getSelectedEntry, restorePrefsDefaults, loadDefaultMods, filterFolders, updateSettings, revertSettings, setLocalStorage, getLocalStorage, getModPrefs, setModPrefs, resetDefaultModPrefs };