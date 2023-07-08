const path = require("path");
const fs = require("fs");
const electron = require("electron");
const app = electron.app || electron.remote.app;

function resolveDirectory(dir) {
    if (dir.toLowerCase().includes("%desktop%")) dir = path.join(app.getPath("desktop"), dir.slice(dir.length - dir.toLowerCase().split(/\/|\\/g).reverse().join("\\").indexOf("%desktop%")));
    return dir.replace(/\%(\w+)\%/g, (match, placeholder) => {
        var env = process.env[placeholder];
        return env ? env : placeholder;
    }).replace(/\\/g, "\\").replace(/\\\\/g, "\\");
}

function readDirectory(dir) {
    dir = resolveDirectory(dir);
    var paths = [];
    for (var filepath of fs.readdirSync(dir)) paths.push({ path: path.join(dir, filepath), name: path.basename(filepath), isDir: isDirectory(path.join(dir, filepath)) });
    return paths;
}

function getExeIconFromPath(dir) {
    const { exec } = require("child_process");
    var command = `powershell.exe -command "[void][Reflection.Assembly]::LoadWithPartialName('System.Drawing'); $icon = [Drawing.Icon]::ExtractAssociatedIcon('${dir}'); $stream = New-Object System.IO.MemoryStream; $icon.ToBitmap().Save($stream, [Drawing.Imaging.ImageFormat]::Png); $base64 = [System.Convert]::ToBase64String($stream.ToArray()); $stream.Dispose(); $base64"`
    return new Promise((resolve, reject) => exec(command, (error, stdout, stderr) => {
        if (stdout) resolve("data:image/png;base64," + stdout.trim());
        else reject(error);
    }));
}

function getIconFromPath(dir) {
    dir = resolveDirectory(dir);
    var ext = path.extname(dir);
    return new Promise((resolve, reject) => {
        if (ext == ".exe") resolve(getExeIconFromPath(dir));
        else if (ext == ".lnk") {
            try {
                var details = electron.shell.readShortcutLink(dir);
                resolve(getExeIconFromPath(details.target));
            }
            catch (err) {
                reject(err);
            }
        }
        else if ([".png", ".jpg", ".jpeg"].includes(ext)) electron.nativeImage.createThumbnailFromPath(dir, { width: 64 }).then((result) => resolve(result.toDataURL())).catch((err) => reject(err));
        else app.getFileIcon(dir).then((icon) => resolve(icon.toDataURL())).catch((err) => reject(err));
    });
}

function openPath(dir) {
    dir = resolveDirectory(dir);
    if (isDirectory(dir)) electron.shell.showItemInFolder(dir);
    else if (isURL(dir)) electron.shell.openExternal(dir);
    else electron.shell.openPath(dir);
}

function pathExists(dir) {
    dir = resolveDirectory(dir);
    return fs.existsSync(dir);
}

function isDirectory(dir) {
    dir = resolveDirectory(dir);
    return fs.lstatSync(dir).isDirectory();
}

function isURL(dir) {
    try {
        new URL(dir);
        return true;
    }
    catch {
        return false;
    }
}

module.exports = { readDirectory, openPath, getIconFromPath, pathExists, isDirectory }