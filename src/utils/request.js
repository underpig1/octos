const source = "https://raw.githubusercontent.com/underpig1/octos-community/master/";
const viewSource = "https://github.com/underpig1/octos-community/tree/master/";
const path = require("path");

function requestWriteFile(url, path) {
    return new Promise((resolve, reject) => fetch(url).then((res) => res.arrayBuffer()).then((data) => {
        require("fs").writeFile(path, Buffer.from(data), (err) => {
            if (err) reject();
            else resolve(path);
        });
    }));
}

function requestFileContent(url) {
    return new Promise((resolve, reject) => fetch(url).then((res) => res.text()).then((data) => resolve(data)));
}

function requestJSONContent(url) {
    return new Promise((resolve, reject) => fetch(url).then((res) => res.json()).then((data) => resolve(data)));
}

function requestSourceModData() {
    return requestJSONContent(path.join(source, "content.json"));
}

function requestModDataByName(name) {
    return new Promise((resolve, reject) => requestSourceModData().then((data) => {
        if (data[name]) resolve(data[name]);
        else reject();
    }));
}

function requestImageDataURL(url) {
    return new Promise((resolve, reject) => fetch(url).then((res) => res.arrayBuffer()).then((data) => {
        var ext = url.split(".").pop();
        var dataURL = `data:image/${ext};base64,` + Buffer.from(data).toString("base64");
        resolve(dataURL);
    }));
}

function requestModImageByName(name) {
    return new Promise((resolve, reject) => requestModDataByName(name).then((data) => {
        var imgpath = data.image;
        if (imgpath) {
            var url = path.join(source, imgpath);
            if (url) resolve(requestImageDataURL(url));
            else reject();
        }
        else reject();
    }).catch(reject));
}

function downloadModByName(name) {
    return new Promise((resolve, reject) => requestModDataByName(name).then((data) => {
        var filepath = data.path;
        var temp = path.join(require("os").tmpdir(), path.basename(filepath));
        if (filepath && temp) resolve(requestWriteFile(path.join(source, filepath), temp));
        else reject();
    }).catch(reject));
}

function addSourceModByName(name, callback) {
    downloadModByName(name).then((filepath) => callback(filepath)).catch(() => callback(false));
}

function goToModSource(name, callback) {
    requestModDataByName(name).then((data) => require("electron").shell.openExternal(path.join(viewSource, data.path)));
}

module.exports = { requestSourceModData, requestModImageByName, addSourceModByName, goToModSource };