// GLOBALS
var focusedIds = { explore: null, modules: null };
var isVisible = true;
var inputGetters = {};
var installedModCards = [];
var exploreMods = {};
var activeTab = "modules";
var modalListener = () => null;
var downloadingCards = [];
var monitorIds;
var modDataBeforeInstalling;
var onInstalled = {};

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

function openIssues() {
    chrome.webview.postMessage({ type: "open-external-link", url: "https://github.com/underpig1/octos/issues" });
}

function openDocumentation() {
    chrome.webview.postMessage({ type: "open-external-link", url: "https://underpig1.github.io/octos/docs/" });
}

function shareMod() {
    chrome.webview.postMessage({ type: "open-external-link", url: "https://underpig1.github.io/octos/docs/?t=publishing" });
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
    modDataBeforeInstalling = JSON.parse(JSON.stringify(userPrefs.modData));
    window.chrome.webview.postMessage({ type: 'install-wallpaper' });
}

// function onWallpaperInstalled() {
//     for (const id of Object.keys(userPrefs.modData)) {
//         if (!modDataBeforeInstalling.hasOwnProperty(id))
//         {
//             const card = document.getElementById(id);
//             if (!card)
//                 return;
//             card.classList.add('loading');
//             setTimeout(() => {
//                 card.classList.remove('loading');
//             }, 2000);
//         }
//     }
//     modDataBeforeInstalling = null;
// }

// MESSAGE LISTENER
window.chrome.webview.addEventListener('message', (e) => {
    const msg = e.data;
    console.log("recieved: ", msg)
    if (msg.type == 'wallpaper-data') {
        handleRecieveModData(msg);
    }
    else if (msg.type == 'error-box') {
        modalDialog(msg.title, msg.caption);
        modalListener = () => { }
    }
    else if (msg.type == 'visibility') {
        isVisible = msg.value;
        updateVisibilityIcon();
    }
    else if (msg.type == 'prefs')
        handleRecieveUserPrefs(msg);
    else if (msg.type == 'monitor-ids') {
        handleRecieveMonitorIds(msg);
    }
    else if (msg.type == 'downloaded-wallpaper')
        onWallpaperDownloaded(msg.id);
    else if (msg.type == 'installed-wallpaper')
        onWallpaperInstalled();
    else if (msg.type == 'navigate-all')
        onNavigateAll(msg.id);
    else if (msg.type == 'request-options')
        sendWallpaperAllOptions(msg['monitor-id']);
    else if (msg.type == 'request-prefs')
        window.chrome.webview.postMessage({type: 'prefs', 'data': userPrefs});
    else if (msg.type == 'wallpaper-loaded')
        handleWallpaperLoaded(msg.entryPath);
    else if (msg.type == 'request')
        handleRequest(msg);
    else if (msg.type == 'command')
        handleCommand(msg);
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

function handleRecieveMonitorIds(msg) {
    monitorIds = msg.data;
    if (activeTab == "modules") {
        const monitorArrow = document.querySelector(".monitor-arrow");
        if (!monitorArrow)
            return;
        if (monitorIds.length > 1) {
            monitorArrow.classList.add('active');
        }
        else {
            monitorArrow.classList.remove('active');
        }
    }
    updateMonitorIndicators();
}

function onNavigateAll(id) {
    for (const monitorId of monitorIds) {
        userPrefs.selected[monitorId] = id;
    }
    updateCardDescription()
    dumpUserPrefs();
    updateMonitorIndicators();
}

function handleWallpaperLoaded(entryPath) {
    console.log('loaded', entryPath);
    for (const monitorId of Object.keys(userPrefs.selected)) {
        const id = userPrefs.selected[monitorId];
        if (id) {
            const modData = userPrefs.modData[id];
            if (modData) {
                if (modData.entryPath.replaceAll('\\', '/') == entryPath) { // found target
                    sendWallpaperAllOptions(monitorId);
                    break;
                }
            }
        }
    }
}

function handleRequest(msg) {
    console.log('recieved request', msg)
    const monitorId = msg['monitor-id'];
    if (!monitorId)
        return;
    const id = userPrefs.selected[monitorId]
    if (!id)
        return;
    const response = {
        "type": "response",
        "requestId": msg.requestId,
        "requestType": msg.requestType
    };
    switch (msg.requestType) {
        case 'options':
            response['data'] = userPrefs.modOptions[id];
            break;
        default:
            break;
    }
    console.log('sending', { type: 'send-to-wallpaper', 'monitor-id': monitorId, data: response });
    window.chrome.webview.postMessage({ type: 'send-to-wallpaper', 'monitor-id': monitorId, data: response });
}

function handleCommand(msg) {
    console.log('recieved command', msg)
    const monitorId = msg['monitor-id'];
    if (!monitorId)
        return;
    const id = userPrefs.selected[monitorId]
    if (!id)
        return;
    switch (msg.commandType) {
        case 'set-option':
            const inputId = msg.data.id;
            const value = msg.data.value;
            if (inputId != null && value != null) {
                console.log('setting')
                userPrefs.modOptions[id][inputId].value = value.toString();
                if (activeTab == 'modules' && focusedIds.modules == id)
                    updateCardDescription();
            }
            break;
        default:
            break;
    }
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
    if (activeTab == name)
        return;
    activeTab = name;
    const scrollBox = document.getElementById(`${activeTab}-scrollbox`)
    if (scrollBox) scrollBox.scrollTo(0, 0);
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
    const nameMap = { "explore": "Gallery", "modules": "Library", "settings": "Preferences" }
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
    focusedIds[activeTab] = el.id;
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
                const id = focusedIds.modules
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
                const id = focusedIds.explore
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
            const focusedId = focusedIds.modules
            document.getElementById(focusedId).classList.add('loading');
            // var focused = false;
            // if (focusedId && userPrefs.selected.length == 1 && Object.keys(userPrefs.selected).includes()) {
            //     for (const id of Object.keys(userPrefs.modData)) {
            //         if (id !=selectedId selectedID) {
            //             focusCard(document.getElementById(id));
            //             selectFocusedCard();
            //             focused = true;
            //             break;
            //         }
            //     }
            // }
            if (Object.values(userPrefs.selected).includes(focusedId)) {
                for (const monitorId of monitorIds) {
                    if (userPrefs.selected[monitorId] == focusedId) {
                        console.log('removing wallpaper associated with ', monitorId);
                        window.chrome.webview.postMessage({ type: "set-wallpaper", url: "", "monitor-id": monitorId });
                    }
                }
            }
            setTimeout(() => {
                const modData = userPrefs.modData[focusedId];
                window.chrome.webview.postMessage({ type: "remove-wallpaper", folderPath: modData.folderPath });
                for (var id of Object.keys(exploreMods)) {
                    if (exploreMods[id] == focusedId) {
                        exploreMods[focusedIds.explore].installed = false;
                        document.getElementById(id).classList.remove("installed");
                    }
                }
            }, 1000);
        }
    }
    modalDialog("Uninstall mod", "Are you sure you want to uninstall this mod?");
}

function updateMods() {
    const modScrollbox = document.getElementById("modules-scrollbox");
    const focusedId = focusedIds.modules;
    installedModCards = {};
    modScrollbox.innerHTML = "";
    for (var child of modScrollbox.children)
        child.classList.remove("focused");
    for (const id of Object.keys(userPrefs.modData)) {
        const data = userPrefs.modData[id]
        var card = createCard(id, data.name, data.author, data.imagePath);
        if (focusedId == id) {
            card.classList.add("focused");
            focusedIds.modules = id;
        }
        modScrollbox.appendChild(card);
        installedModCards[id] = card;
        for (const monitorId of monitorIds) {
            if (userPrefs.selected[monitorId] == id) {
                window.chrome.webview.postMessage({ type: 'set-wallpaper', url: data.entryPath, 'monitor-id': monitorId });
            }
        }
    }
    var focusCheck = false;
    for (var mod of modScrollbox.childNodes) focusCheck = focusCheck || mod.classList.contains("focused");
    if (!focusCheck) {
        var firstCard = modScrollbox.querySelector(".card");
        if (firstCard) {
            firstCard.classList.add("focused");
            focusedIds.modules = firstCard.id;
        }
        else return;
    }
    var uploadCard = document.getElementById("card-upload-template").content.cloneNode(true).firstElementChild;
    add3dEffect(uploadCard);
    modScrollbox.appendChild(uploadCard);
    updateCardDescription();
    updateDownloadedMods();
    updateResponsiveElements();
    updateMonitorIndicators();
}

function updateMonitorIndicators() {
    for (const id of Object.keys(userPrefs.modData)) {
        const card = document.getElementById(id);
        if (card) {
            const indicators = card.querySelector('.indicator-container');
            if (!indicators)
                return;
            indicators.innerHTML = "";
            function addIndicator(monitorId) {
                const div = document.createElement('div');
                div.classList.add('monitor-indicator');
                div.innerText = cleanMonitorId(monitorId);
                indicators.appendChild(div);
            }
            var allMonitors = true;
            for (const monitorId of monitorIds) {
                if (userPrefs.selected[monitorId] != id)
                    allMonitors = false;
            }
            if (allMonitors && monitorIds.length == 1) addIndicator(monitorIds[0]);
            else {
                for (const monitorId of monitorIds) {
                    if (userPrefs.selected[monitorId] == id) addIndicator(monitorId);
                }
            }
        }
    }
}

function selectFocusedCard(monitorId = null) {
    console.log('SELECTING')
    const focusedId = focusedIds.modules
    if (focusedId) {
        const modScrollbox = document.getElementById("modules-scrollbox");
        const focusedCard = document.getElementById(focusedId);
        if (focusedCard) {
            // for (var child of modScrollbox.children) child.classList.remove("selected");
            // focusedCard.classList.add("selected");

            const entryPath = userPrefs.modData[focusedId].entryPath;
            if (monitorId) {
                userPrefs.selected[monitorId] = focusedCard.id;
                window.chrome.webview.postMessage({ type: 'set-wallpaper', url: entryPath, "monitor-id": monitorId });
            }
            else {
                for (const monitorId of monitorIds) {
                    userPrefs.selected[monitorId] = focusedCard.id;
                    window.chrome.webview.postMessage({ type: 'set-wallpaper', url: entryPath, "monitor-id": monitorId });
                }
            }
            dumpUserPrefs();
            updateMonitorIndicators();

            var buttonDisabled = true;
            for (const monitorId of monitorIds) {
                if (userPrefs.selected[monitorId] != focusedId) {
                    buttonDisabled = false;
                    break;
                }
            }
            if (buttonDisabled) {
                const button = document.getElementById('modules-card-description').querySelector("button");
                button.classList.add("inactive");
            }
        }
    }
}

function handleExitMonitorSelector(e) {
    if (e.target.id != 'monitor-selector' && !e.target.classList.contains('monitor-item')) {
        document.getElementById('monitor-selector').classList.remove('active');
    }
}
function cleanMonitorId(monitorId) {
    return monitorId
        .replaceAll('\\', '').replaceAll('.', ' ').trim();
}

function openMonitorSelector(e) {
    e.stopPropagation();
    document.body.addEventListener('click', handleExitMonitorSelector);
    const monitorSelector = document.getElementById('monitor-selector');
    if (!monitorSelector)
        return;
    monitorSelector.innerHTML = "";
    for (const monitorId of monitorIds) {
        const div = document.createElement('div');
        div.classList.add('monitor-item');
        div.setAttribute("onclick", "selectMonitor(event, this)");
        div.innerText = "Assign to " + cleanMonitorId(monitorId);
        div.id = monitorId;
        if (userPrefs.selected[monitorId] == focusedIds.modules)
            div.classList.add('inactive');
        monitorSelector.appendChild(div);
    }
    monitorSelector.classList.add('active');
    const rect = monitorSelector.getBoundingClientRect();
    const x = e.clientX - rect.width;
    const y = e.clientY - rect.height - 5;
    monitorSelector.style.transform = `translate(${x}px, ${y}px)`;
}

function selectMonitor(e, el) {
    const monitorSelector = document.getElementById('monitor-selector');
    monitorSelector.classList.remove('active');
    document.body.removeEventListener('click', handleExitMonitorSelector);
    const monitorId = el.id;
    if (monitorId)
        selectFocusedCard(monitorId);
}

// CARD DESCRIPTION
function updateCardDescription() {
    cleanFocus();
    if (activeTab == 'modules') {
        const id = focusedIds.modules
        if (id) {
            const cardData = userPrefs.modData[id]
            if (cardData)
                setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, userPrefs.modOptions[id], true);
        }
    }
    else if (activeTab == 'explore') {
        const id = focusedIds.explore
        if (id) {
            const cardData = exploreMods[id]
            if (cardData)
                setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, null, cardData.installed, cardData.previewPath);
        }
    }
}

function setCardDescription(prefix = "explore", title = "", author = "", description = "", options = null, installed = false, preview = null) {
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
    if (prefix == "modules") {
        for (const monitorId of monitorIds) {
            if (userPrefs.selected[monitorId] != focusedIds.modules) {
                buttonDisabled = false;
                break;
            }
        }
    }
    else if (prefix == "explore") {
        const previewImage = cardDescription.querySelector(".preview");
        const previewModalImage = document.getElementById("modal-container").querySelector("img");
        if (previewImage && previewModalImage) {
            if (preview) {
                const bkImg = `url("${preview}")`;
                previewImage.classList.add('active');
                if (previewImage.style.backgroundImage != bkImg) {
                    previewImage.classList.remove('loaded');
                    previewImage.style.backgroundImage = "";
                    const img = new Image();
                    img.src = preview;
                    previewModalImage.src = preview;
                    img.onload = () => {
                        previewImage.classList.add('loaded');
                        previewImage.style.backgroundImage = bkImg;
                    }
                }
            }
            else {
                previewImage.style.backgroundImage = `none`;
                previewImage.classList.remove('active');
            }
        }
        const card = document.getElementById(focusedIds[prefix]);
        if (downloadingCards.includes(focusedIds[prefix])) {
            buttonDisabled = true;
            cardDescription.querySelector("button").innerText = "Downloading...";
            card.classList.add('loading')
        }
        else if (installed === false) {
            buttonDisabled = false;
            cardDescription.querySelector("button").innerText = "Download";
            card.classList.remove('loading')
        }
        else {
            buttonDisabled = true;
            cardDescription.querySelector("button").innerText = "Downloaded";
            card.classList.remove('loading')
        }
    }
    setButtonDisabled(buttonDisabled);

    if (prefix == "modules") {
        const monitorArrow = cardDescription.querySelector(".monitor-arrow");
        if (!monitorArrow)
            return;
        if (monitorIds.length > 1) {
            monitorArrow.classList.add('active');
        }
        else {
            monitorArrow.classList.remove('active');
        }
    }
}

// CARD OPTIONS
function createInput(options, id) {
    console.log('creating input from ', options)
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
        const id = focusedIds.modules
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
    const id = focusedIds.modules;
    if (id) {
        const modData = userPrefs.modData[id];
        if (modData) {
            for (const inputId of Object.keys(inputGetters)) {
                const value = getInput(inputId);
                if (userPrefs.modOptions[id].hasOwnProperty(inputId)) {
                    if (userPrefs.modOptions[id][inputId].value != value) {
                        userPrefs.modOptions[id][inputId].value = value;
                        for (const monitorId of Object.keys(userPrefs.selected)) {
                            if (userPrefs.selected[monitorId] == id) {
                                const payload = { 'type': 'event', 'eventType': 'options-change', 'data': {'id': inputId, 'value': value } };
                                window.chrome.webview.postMessage({
                                    type: 'send-to-wallpaper',
                                    'monitor-id': monitorId,
                                    data: payload
                                });
                            }
                        }
                    }
                }
            }
            dumpUserPrefs()
        }
    }
}

function sendWallpaperAllOptions(monitorId) {
    const id = userPrefs.selected[monitorId];
    const options = userPrefs.modOptions[id];
    for (const inputId of Object.keys(options))
    {
        const payload = { 'type': 'event', 'eventType': 'options-load', 'data': {'id': inputId, 'value': options[inputId].value } };
        window.chrome.webview.postMessage({
            type: 'send-to-wallpaper',
            'monitor-id': monitorId,
            data: payload
        });
    }
}

// MODAL
function handleModalClickOut(e) {
    if (e.target.id == 'modal-container') {
        const modal = document.getElementById("modal-container");
        modal.classList.remove("active");
    }
}

function modalDialog(title = "Modal title", description = "Modal description", textbox = null) {
    const modal = document.getElementById("modal-container");
    modal.querySelector(".modal-title").innerText = title;
    modal.querySelector(".modal-description").innerText = description;
    modal.classList.add("active");
    modal.classList.remove("preview-modal");
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

function modalPreview() {
    const modal = document.getElementById("modal-container");
    modal.classList.add("active");
    modal.classList.add("preview-modal");
}

// USER PREFERENCES
var userPrefs = {}
const appOptionsDefaults = {
    "disableMouseInput": false, "memorySaver": true, "runOnStartup": true, "enableGPU": true
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
    var id = focusedIds.explore;
    if (!focusedIds.explore)
        return;
    const modData = exploreMods[id];
    downloadingCards.push(id);
    updateCardDescription();
    setTimeout(() => window.chrome.webview.postMessage({ type: 'download-wallpaper', url: modData.zipPath, id }), 1000);
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
    const id = focusedIds.explore;
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
    const resolvePath = (rel) => rel ? ('https://raw.githubusercontent.com/underpig1/octos-community/refs/heads/master/' + rel).replace('\\', '/') : null;
    const resolveCookedPath = (rel) => rel ? ('https://github.com/underpig1/octos-community/tree/master/' + rel).replace('\\', '/') : null;
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
                    exploreMods["explore-" + name] = { name, author: modData.author, description: modData.description, folderPath: resolveCookedPath(modData.folderPath), zipPath: resolvePath(modData.zipPath), previewPath: resolvePath(modData.previewPath) }
                }
                updateDownloadedMods();
                cleanFocus();
                updateCardDescription();
            })();
        })
        .catch(err => console.error('Failed to load index:', err));
}