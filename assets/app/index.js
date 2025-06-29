// GLOBALS
var focusedIDs = { explore: null, modules: null };
var selectedID;
var isVisible = true;
var inputGetters = {};
var installedModCards = [];
var exploreMods = {};
var activeTab = "modules";
var modalListener = () => null;
var downloadingCards = [];
var monitorIds;

// WINDOW ACTIONS
function handleWindowDrag(e) {
    if (e.target === e.currentTarget)
        window.chrome.webview.postMessage({ type: "drag" });
}
document.getElementById('topbar').addEventListener('mousedown', handleWindowDrag);

function exit() {
    window.chrome.webview.postMessage({ type: "close" })
}

function minimize() {
    window.chrome.webview.postMessage({ type: 'minimize' })
}

function maximize() {
    window.chrome.webview.postMessage({ type: 'maximize' })
}

function openModsFolder() {
    chrome.webview.postMessage({ type: "open-folder", path: "wallpapers/", relative: true });
}

function findMoreMods() {
    chrome.webview.postMessage({ type: "open-external-link", url: "https://github.com/underpig1/octos-community" });
}

function openGitHub() {
    chrome.webview.postMessage({ type: "open-external-link", url: "https://github.com/underpig1/octos" });
}

function openDocumentation() {
    chrome.webview.postMessage({ type: "open-external-link", url: "https://underpig1.github.io/octos/docs/" });
}

function refresh() {
    chrome.webview.postMessage({ type: "refresh" });
    // chrome.webview.postMessage({ type: "request-wallpaper-data" });
}

function toggleVisibility() {
    isVisible = !isVisible;
    updateVisibilityIcon();
    window.chrome.webview.postMessage({ type: 'set-visibility', value: isVisible });
}

function uploadMod() {
    window.chrome.webview.postMessage({ type: 'install-wallpaper' });
}

// MESSAGE LISTENER
window.chrome.webview.addEventListener('message', (e) => {
    const msg = e.data;
    console.log("recieved: ", msg)
    if (msg.type == 'wallpaper-data') {
        handleRecieveModData(msg);
    }
    else if (msg.type == 'error-box')
        modalDialog(msg.title, msg.caption);
    else if (msg.type == 'visibility') {
        isVisible = msg.value;
        updateVisibilityIcon();
    }
    else if (msg.type == 'prefs')
        handleRecieveUserPrefs(msg);
    else if (msg.type == 'monitor-ids') {
        monitorIds = msg.data
    }
    else if (msg.type == 'downloaded-wallpaper')
        onWallpaperDownloaded(msg.id);
});

// MOD DATA
function handleRecieveModData(msg) {
    if (!msg.data) return;
    const modData = msg.data
        .filter(x => x.hasOwnProperty('folderPath')).sort((a, b) => a.name - b.name)
        .reduce((acc, curr) => {
            acc[curr.folderPath] = curr;
            return acc;
        }, {});
    userPrefs.modData = modData;
    if (!userPrefs.hasOwnProperty('modOptions')) userPrefs.modOptions = {}
    for (const id of Object.keys(modData))
        if (!userPrefs.modOptions.hasOwnProperty(id)) userPrefs.modOptions[id] = modData[id].options
        else {
            const existingOptions = userPrefs.modOptions[id]
            const newOptions = modData[id].options
            Object.keys(newOptions).forEach(k => { if (!existingOptions.hasOwnProperty(k)) userPrefs.modOptions[id][k] = newOptions[k] });
        }
    updateMods();
}

// INITIALIZATION SCRIPT
document.addEventListener("DOMContentLoaded", () => {
    for (var range of document.getElementsByClassName("range-input")) updateRange(range);
    for (var img of document.getElementsByTagName("img")) img.draggable = false;

    window.chrome.webview.postMessage({ type: 'request-prefs' });
    window.chrome.webview.postMessage({ type: 'request-monitor-ids' });
    window.chrome.webview.postMessage({ type: 'request-wallpaper-data' });

    populateExplore();
    retrieveWorking();
    setInterval(handleScrollShadows, 100);
    window.onresize();
});

// STYLING
function updateResponsiveElements() {
    var computed = window.getComputedStyle(document.querySelector(".card"));
    const getCSSVariable = (prop) => parseInt(window.getComputedStyle(document.body).getPropertyValue(prop));
    // var cardWidth = parseInt(computed.getPropertyValue("width")) + parseInt(computed.getPropertyValue("margin-left")) + parseInt(computed.getPropertyValue("margin-right"));
    // var totalSpace = document.querySelector(".content").clientWidth;
    // var scrollboxWidth = totalSpace - getCSSVariable("--default-sidebar-width");
    // var numberOfCards = Math.floor(scrollboxWidth/cardWidth);
    // var newSidebarWidth = totalSpace - numberOfCards*cardWidth - 10;
    // document.documentElement.style.setProperty("--sidebar-width", newSidebarWidth + "px");

    var scrollboxWidth = document.querySelector(".content").clientWidth - getCSSVariable("--sidebar-width");
    var cardWidth = getCSSVariable("--default-card-width") + parseInt(computed.getPropertyValue("margin-left")) + parseInt(computed.getPropertyValue("margin-right"));
    document.documentElement.style.setProperty("--card-width", scrollboxWidth / Math.floor(scrollboxWidth / cardWidth) - 20 + "px");
    for (var shadow of document.getElementsByClassName("scroll-shadow")) shadow.style.width = scrollboxWidth + "px";
}

window.onresize = updateResponsiveElements;

function handleScrollShadows() {
    var top = document.getElementById("scroll-shadow-top");
    var bottom = document.getElementById("scroll-shadow-bottom");
    var content = document.querySelector(".tab-content.active > .scrollbox");

    if (content) {
        top.style.opacity = 0;
        bottom.style.opacity = 0;
        if (content.clientHeight < content.scrollHeight) {
            var minScrollPos = content.scrollTop < 1;
            var maxScrollPos = Math.abs(content.scrollHeight - content.clientHeight - content.scrollTop) < 1;
            if (!minScrollPos) top.style.opacity = 1;
            if (!maxScrollPos) bottom.style.opacity = 1;
        }
    }
    else {
        top.style.opacity = 0;
        bottom.style.opacity = 0;
    }
}

function setContent(el) {
    var name = el.getAttribute("id");
    activeTab = name;
    var target = document.getElementById(name + "-tab");
    var tabs = document.getElementsByClassName("tab-content");
    if (target) {
        for (var tab of tabs) {
            if (tab.classList.contains("active")) {
                if (target == tab) return;
                tab.classList.remove("active");
            }
        }
        target.classList.add("active");
    }

    var title = document.getElementById("title");
    title.style.opacity = 0;
    const nameMap = { "explore": "Gallery", "modules": "Library", "settings": "Settings" }
    setTimeout(() => title.textContent = nameMap[name], 100);
    setTimeout(() => title.style.opacity = 1, 100);

    var highlight = document.querySelector(".nav-highlight");
    highlight.style.setProperty("--nav-active", Array.prototype.indexOf.call(el.parentNode.children, el) - 1);
    cleanFocus();
    updateCardDescription();
}

function updateVisibilityIcon() {
    var visibleIcon = document.getElementById("visible-icon");
    if (isVisible) visibleIcon.src = "img/visibility.png";
    else visibleIcon.src = "img/invisible.png";
}

// CARD ACTIONS
function focusCard(el) {
    for (var card of el.parentNode.getElementsByClassName("card")) card.classList.remove("focused");
    el.classList.add("focused");
    focusedIDs[activeTab] = el.id;
    updateCardDescription();
}

function cleanFocus() {
    const cardDescription = document.getElementById(activeTab + "-card-description");
    if (cardDescription) {
        if (activeTab == 'modules') {
            if (installedModCards.length == 0)
                cardDescription.classList.add("inactive");
            else {
                cardDescription.classList.remove("inactive");
                const id = focusedIDs.modules
                if (!id || !document.getElementById(id)) {
                    const candidate = document.getElementById('mod-scrollbox').querySelector('.card');
                    if (candidate)
                        focusCard(candidate);
                }
            }
        }
        else if (activeTab == 'explore') {
            if (Object.keys(exploreMods).length == 0)
                cardDescription.classList.add("inactive");
            else {
                cardDescription.classList.remove("inactive");
                const id = focusedIDs.explore
                if (!id || !document.getElementById(id)) {
                    const candidate = document.getElementById('explore-scrollbox').querySelector('.card');
                    if (candidate)
                        focusCard(candidate);
                }
            }
        }
    }
}

function calc3dEffect(e) {
    const clamp = (x, t) => Math.min(Math.max(x, -t), t)
    const { clientX, clientY, currentTarget } = e;
    const { clientWidth, clientHeight } = currentTarget;
    const rect = currentTarget.getBoundingClientRect();

    const x = clientX - rect.left - clientWidth / 2;
    const y = clientY - rect.top - clientHeight / 2;

    const rotateX = clamp(-y / e.target.offsetWidth * 20, 10);
    const rotateY = clamp(x / e.target.offsetHeight * 20, 10);
    return { rotateX, rotateY }
}

function add3dEffect(div) {
    div.onmousemove = (e) => {
        let { rotateX, rotateY } = calc3dEffect(e)
        e.currentTarget.querySelector('.card-content').style.transform = ` perspective(500px) rotateX(${rotateX.toFixed(2)}deg) rotateY(${rotateY.toFixed(2)}deg) scale(0.95)`;
    }
    div.onmouseleave = (e) => {
        e.currentTarget.querySelector('.card-content').style.transform = `perspective(500px) rotateX(0) rotateY(0) scale(1)`, 100;
    }
    div.onmousedown = (e) => {
        let { rotateX, rotateY } = calc3dEffect(e)
        e.currentTarget.querySelector('.card-content').style.transform = ` perspective(500px) rotateX(${rotateX.toFixed(2)}deg) rotateY(${rotateY.toFixed(2)}deg) scale(0.92)`;
    }
    div.onmouseup = (e) => {
        let { rotateX, rotateY } = calc3dEffect(e)
        e.currentTarget.querySelector('.card-content').style.transform = ` perspective(500px) rotateX(${rotateX.toFixed(2)}deg) rotateY(${rotateY.toFixed(2)}deg) scale(0.95)`;
    }
}

function createCard(id = "", title = "Mod name", author = null, backgroundImage = "") {
    var template = document.getElementById("card-template");
    var div = template.content.cloneNode(true).firstElementChild;
    div.querySelector(".card-title").innerText = title;
    div.querySelector(".card-author").innerText = author ? "By " + author : "";
    div.id = id;
    if (backgroundImage) div.querySelector('.card-content').style.backgroundImage = `url('${backgroundImage.replace(/\\/g, "/")}')`;
    add3dEffect(div)
    return div;
}

function removeFocusedCard() {
    modalListener = (state) => {
        if (state) {
            const focusedId = focusedIDs.modules
            if (focusedId && focusedId == selectedID) {
                for (const id of Object.keys(userPrefs.modData)) {
                    if (id != selectedID) {
                        focusCard(document.getElementById(id));
                        selectFocusedCard();
                        break;
                    }
                }
            }
            const modData = userPrefs.modData[focusedId];
            window.chrome.webview.postMessage({ type: "remove-wallpaper", folderPath: modData.folderPath });
            for (var id of Object.keys(exploreMods)) {
                if (exploreMods[id] == focusedId) {
                    exploreMods[focusedIDs.explore].installed = false;
                    document.getElementById(id).classList.remove("installed");
                }
            }
        }
    }
    modalDialog("Uninstall mod", "Are you sure you want to uninstall this mod?");
}

function updateMods() {
    const modScrollbox = document.getElementById("modules-scrollbox");
    const focusedId = focusedIDs.modules;
    installedModCards = {};
    modScrollbox.innerHTML = "";
    var changeFocused = JSON.parse(JSON.stringify(!selectedID))
    console.log('READY TO CHANGE FOCUS???', changeFocused)
    for (const id of Object.keys(userPrefs.modData)) {
        const data = userPrefs.modData[id]
        var card = createCard(id, data.name, data.author, data.imagePath);
        if (focusedId == id) {
            for (var child of modScrollbox.children) child.classList.remove("focused");
            card.classList.add("focused");
            focusedIDs.modules = id;
        }
        if (userPrefs.selected == id) {
            for (var child of modScrollbox.children) child.classList.remove("selected");
            card.classList.add("selected");
            selectedID = id;
        }
        modScrollbox.appendChild(card);
        installedModCards[id] = card;
    }
    var focusCheck = false;
    for (var mod of modScrollbox.childNodes) focusCheck = focusCheck || mod.classList.contains("focused");
    if (!focusCheck) {
        var firstCard = modScrollbox.querySelector(".card");
        firstCard.classList.add("focused");
        focusedIDs.modules = firstCard.id;
    }
    var uploadCard = document.getElementById("card-upload-template").content.cloneNode(true).firstElementChild;
    add3dEffect(uploadCard);
    modScrollbox.appendChild(uploadCard);
    updateCardDescription();
    updateDownloadedMods();
    updateResponsiveElements();
    if (changeFocused)
        selectFocusedCard();
}

function selectFocusedCard() {
    console.log('SELECTING')
    const focusedId = focusedIDs.modules
    if (focusedId) {
        const modScrollbox = document.getElementById("modules-scrollbox");
        const focusedCard = document.getElementById(focusedId);
        if (focusedCard) {
            for (var child of modScrollbox.children) child.classList.remove("selected");
            focusedCard.classList.add("selected");

            selectedID = focusedCard.id;
            const entryPath = userPrefs.modData[focusedId].entryPath;
            window.chrome.webview.postMessage({ type: 'set-wallpaper', url: entryPath, "monitor-id": monitorIds[0] });
            userPrefs.selected = selectedID;
            dumpUserPrefs();

            const button = document.getElementById('modules-card-description').querySelector("button");
            button.disabled = true;
            button.classList.add("inactive");
        }
    }
}

// CARD DESCRIPTION
function updateCardDescription() {
    cleanFocus();
    if (activeTab == 'modules') {
        const id = focusedIDs.modules
        if (id) {
            const cardData = userPrefs.modData[id]
            if (cardData)
                setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, userPrefs.modOptions[id], true);
        }
    }
    else if (activeTab == 'explore') {
        const id = focusedIDs.explore
        if (id) {
            const cardData = exploreMods[id]
            console.log(exploreMods[id].installed);
            if (cardData)
                setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, null, cardData.installed);
        }
    }
}

function setCardDescription(prefix = "explore", title = "", author = "", description = "", options = null, installed = false, source = null) {
    const cardDescription = document.getElementById(prefix + "-card-description");
    cardDescription.querySelector(".title-content").innerText = title;
    if (author) cardDescription.querySelector(".author").innerHTML = `By <a class="author-content">${author}</a>`;
    else cardDescription.querySelector(".author").innerHTML = "";
    cardDescription.querySelector(".description").innerText = description;
    if (options) {
        setCardOptions(options);
        cardDescription.classList.remove("empty");
    }
    else {
        setCardOptions({});
        cardDescription.classList.add("empty");
    }

    setButtonDisabled = (state) => {
        var button = cardDescription.querySelector("button");
        button.disabled = state;
        if (state) button.classList.add("inactive");
        else button.classList.remove("inactive");
    }

    var buttonDisabled = true;
    if (selectedID == focusedIDs.modules && prefix == "modules") buttonDisabled = true;
    else buttonDisabled = false;
    if (prefix == "explore") {
        if (downloadingCards.includes(focusedIDs[prefix])) {
            buttonDisabled = true;
            cardDescription.querySelector("button").innerText = "Downloading...";
        }
        else if (installed === false) {
            buttonDisabled = false;
            cardDescription.querySelector("button").innerText = "Download";
        }
        else {
            buttonDisabled = true;
            cardDescription.querySelector("button").innerText = "Downloaded";
        }
    }
    setButtonDisabled(buttonDisabled);
}

// CARD OPTIONS
function createInput(options, id) {
    const cardOptions = document.getElementById("card-options");
    var type = options.type;
    var getter = () => null;
    if (!type) type = "checkbox";
    if (type == "color-picker") type = "color";
    if (options.description && !options.label) options.label = options.description;
    if (!options.label) return false;
    if (type == "dropdown") type = "select";
    var template = document.getElementById(type + "-input");
    if (template) {
        var div = template.content.cloneNode(true).firstElementChild;
        if (options.label) div.querySelector("p").innerText = options.label;
        var input = div.querySelector("input");
        if (options.value) input.setAttribute("value", options.value);
        getter = () => input.value;
        if (type == "checkbox") {
            if (options.value) input.checked = options.value;
            getter = () => input.checked;
        }
        else if (type == "file") getter = () => input.files[0];
        else if (type == "description") {
            if (options.value) div.querySelector("p").innerText = options.value;
            getter = () => null;
        }
        else if (type == "select") {
            var select = div.querySelector("select");
            if (options.options) {
                for (var option of options.options) {
                    var el = document.createElement("option");
                    el.innerText = option;
                    el.value = option;
                    select.appendChild(el);
                }
            }
            getter = () => select.options[select.selectedIndex].text;
        }
        else if (type == "range" || type == "number") {
            if (options.min) input.setAttribute("min", options.min);
            if (options.max) input.setAttribute("max", options.max);
            if (options.step) input.setAttribute("step", options.step);
            if (type == "range") updateRange(input);
        }
        inputGetters[id] = getter;
        cardOptions.appendChild(div);
    }
    return getter;
}

function getInput(id) {
    if (Object.keys(inputGetters).includes(id)) {
        return inputGetters[id]();
    }
    return null;
}

function updateRange(el) {
    var input = el;
    var p = el.parentNode.querySelector(".range-value");
    p.innerText = input.value;
}

function setCardOptions(json) {
    const cardOptions = document.getElementById("card-options");
    cardOptions.innerHTML = "";
    inputGetters = {};
    var candidates = [];
    for (var id of Object.keys(json))
        candidates.push(createInput(json[id], id));
    if (candidates.every(x => !x)) {
        const cardDescription = document.getElementById(activeTab + "-card-description");
        cardDescription.classList.add("empty");
        return;
    }
    if (Object.keys(json).length > 0) {
        function createScrollShadow(bottom = false) {
            let scrollShadow = document.createElement("div");
            scrollShadow.classList.add('scroll-shadow');
            cardOptions.appendChild(scrollShadow);
            scrollShadow.style.zIndex = '5';
            scrollShadow.style.height = '15px';
            scrollShadow.style.left = '-5px';
            scrollShadow.style.width = 'calc(100% + 10px)';
            scrollShadow.style.position = 'absolute';
            scrollShadow.style.top = '0';
            scrollShadow.style.pointerEvents = 'none';
            // scrollShadow.style.backgroundColor = 'red'
            return scrollShadow;
        }

        const top = createScrollShadow(false);
        const bottom = createScrollShadow(true);
        const handleCardOptionScrollShadows = () => {
            top.style.opacity = 0;
            bottom.style.opacity = 0;
            top.style.top = cardOptions.scrollTop + 'px';
            bottom.style.top = cardOptions.scrollTop + cardOptions.clientHeight - bottom.clientHeight - 1 + 'px';
            if (cardOptions.clientHeight < cardOptions.scrollHeight) {
                var minScrollPos = cardOptions.scrollTop < 1;
                var maxScrollPos = Math.abs(cardOptions.scrollHeight - cardOptions.clientHeight - cardOptions.scrollTop) < 1;
                if (!minScrollPos) top.style.opacity = 1;
                if (!maxScrollPos) bottom.style.opacity = 1;
            }
            else {
                top.style.opacity = 0;
                bottom.style.opacity = 0;
            }
        }
        cardOptions.addEventListener('scroll', handleCardOptionScrollShadows);
        window.addEventListener('resize', handleCardOptionScrollShadows);
        handleCardOptionScrollShadows();

        const restorePrefs = document.createElement('a')
        restorePrefs.setAttribute('id', 'restore-preferences')
        restorePrefs.onclick = restoreModOptions
        restorePrefs.innerText = 'Restore default mod options'
        cardOptions.appendChild(restorePrefs);
    }
}

function restoreModOptions() {
    modalDialog("Restore mod options", "Are you sure you want to restore default mod options? This action is irreversible.");
    modalListener = (state) => {
        const id = focusedIDs.modules
        if (id && state) {
            const modData = userPrefs.modData[id]
            if (modData) {
                const defaultOptions = modData.options;
                if (defaultOptions) {
                    userPrefs.modOptions[id] = defaultOptions;
                    updateCardDescription();
                    dumpUserPrefs();
                }
            }
        }
    }
}

function updateCardOptions() {
    const id = focusedIDs.modules
    if (id) {
        const modData = userPrefs.modData[id];
        if (modData) {
            for (const inputId of Object.keys(inputGetters)) {
                const value = getInput(inputId);
                if (userPrefs.modOptions[id].hasOwnProperty(inputId))
                    userPrefs.modOptions[id][inputId].value = value;
            }
            dumpUserPrefs()
        }
    }
}

// MODAL
function modalDialog(title = "Modal title", description = "Modal description", textbox = null) {
    const modal = document.getElementById("modal-container");
    modal.querySelector(".modal-title").innerText = title;
    modal.querySelector(".modal-description").innerText = description;
    modal.classList.add("active");
    if (textbox) {
        modal.classList.add("textbox");
        modal.querySelector(".modal-textbox").value = textbox;
    }
    else modal.classList.remove("textbox");
}

function modalResponse(state) {
    const modal = document.getElementById("modal-container");
    modal.classList.remove("active");
    modalListener(state, modal.querySelector(".modal-textbox").value);
}

// USER PREFERENCES
var userPrefs = {}
const appOptionsDefaults = {
    "disableMouseInput": false, "memorySaver": false, "runOnStartup": true
}

function dumpUserPrefs() {
    window.chrome.webview.postMessage({ type: 'dump-prefs', value: userPrefs });
}

function restoreAppOptions() {
    modalDialog("Restore app settings", "Are you sure you want to restore default app settings?");
    modalListener = (state) => {
        if (state) {
            userPrefs.appOptions = JSON.parse(JSON.stringify(appOptionsDefaults))
            for (const id of Object.keys(userPrefs.appOptions)) {
                const el = document.getElementById(id)
                if (el)
                    if (el.type == "checkbox") el.checked = userPrefs.appOptions[id];
                    else el.value = userPrefs.appOptions[id]
            }
            dumpUserPrefs();
        }
    }
}

function updateAppOptions(el) {
    if (el.type == "checkbox") var value = el.checked;
    else var value = el.value;
    if (!userPrefs.hasOwnProperty('appOptions'))
        userPrefs.appOptions = {}
    userPrefs.appOptions[el.id] = value;
    dumpUserPrefs();
}

function handleRecieveUserPrefs(msg) {
    userPrefs = msg.data
    console.log('recieved prefs', userPrefs);

    var inputs = document.getElementsByClassName("settings-input");
    if (!userPrefs.hasOwnProperty('appOptions'))
        return;
    for (var el of inputs) {
        console.log(el, el.id);
        if (userPrefs.appOptions[el.id] != null) {
            if (el.type == "checkbox") el.checked = userPrefs.appOptions[el.id];
            else el.value = userPrefs.appOptions[el.id];
        }
    }
}

// EXPLORE
function downloadFocusedCard() {
    var id = focusedIDs.explore;
    if (!focusedIDs.explore)
        return;
    const modData = exploreMods[id];
    downloadingCards.push(id);
    updateCardDescription();
    window.chrome.webview.postMessage({ type: 'download-wallpaper', url: modData.zipPath, id });
}

function onWallpaperDownloaded(id) {
    console.log('downloaded', id);
    downloadingCards.splice(downloadingCards.indexOf(id), 1);
    exploreMods[id].installed = true;
    document.getElementById(id).classList.add("installed");
    console.log(exploreMods[id].installed);
    updateCardDescription();
}

function goToSource() {
    const id = focusedIDs.explore;
    if (id) {
        const modData = exploreMods[id];
        var url = modData.folderPath
        if (modData.website)
            url = modData.website;
        else if (modData.source)
            url = modData.source;
        window.chrome.webview.postMessage({ type: 'open-external-link', url });
    }
}

function updateDownloadedMods() {
    for (var id of Object.keys(exploreMods)) {
        var modData = exploreMods[id];
        var installed = false;
        for (const id of Object.keys(userPrefs.modData)) {
            const data = userPrefs.modData[id];
            if (data.name == modData.name && data.description == modData.description && data.author == modData.author) {
                installed = true;
                break;
            }
        }
        modData.installed = installed;
        const card = document.getElementById(id);
        if (installed) card.classList.add("installed");
        else card.classList.remove("installed");
    }
}

function populateExplore() {
    const resolvePath = (rel) => ('https://raw.githubusercontent.com/underpig1/octos-community/refs/heads/master/' + rel.replace('\\', '/'));
    const resolveCookedPath = (rel) => ('https://github.com/underpig1/octos-community/tree/master/' + rel.replace('\\', '/'));
    fetch(resolvePath('index.json'))
        .then(res => res.json())
        .then(data => {
            const exploreScrollbox = document.getElementById("explore-scrollbox");
            exploreScrollbox.innerHTML = "";
            (async () => {
                for (var i = 0, p = Promise.resolve(); i < data.length; i++) {
                    const modData = data[i];
                    const name = modData.name;
                    const imagePath = resolvePath(modData.imagePath);
                    var card = createCard("explore-" + name, modData.name, modData.author, imagePath);
                    exploreScrollbox.appendChild(card);
                    exploreMods["explore-" + name] = { name, author: modData.author, description: modData.description, folderPath: resolveCookedPath(modData.folderPath), zipPath: resolvePath(modData.zipPath) }
                }
                updateDownloadedMods();
                cleanFocus();
                updateCardDescription();
            })();
        })
        .catch(err => console.error('Failed to load index:', err));
}