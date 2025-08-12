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
function reload() {
    chrome.webview.postMessage({ type: "reload" });
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
        window.chrome.webview.postMessage({ type: 'prefs', 'data': userPrefs });
    else if (msg.type == 'wallpaper-loaded')
        handleWallpaperLoaded(msg['monitor-id']);
    else if (msg.type == 'request')
        handleRequest(msg);
    else if (msg.type == 'command')
        handleCommand(msg);
    else if (msg.type == 'config-data')
        handleConfigData(msg.data);
    else if (msg.type == 'preview')
        handleOnPreview(msg.data);
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
    for (const id of Object.keys(modData)) {
        if (!userPrefs.modOptions.hasOwnProperty(id)) userPrefs.modOptions[id] = modData[id].options
        else {
            const existingOptions = userPrefs.modOptions[id]
            const newOptions = modData[id].options
            Object.keys(newOptions).forEach(k => { if (!existingOptions.hasOwnProperty(k)) userPrefs.modOptions[id][k] = newOptions[k] });
        }
    }
    if (previewModData) {
        appendModData(previewModData);
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

function handleWallpaperLoaded(monitorId) {
    if (!monitorId || !userPrefs.selected)
        return;
    console.log('loaded', monitorId);
    for (const mid of Object.keys(userPrefs.selected)) {
        if (mid == monitorId) {
            sendWallpaperAllOptions(monitorId);
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

function unsetByMonitorId(monitorId, send = true) {
    if (userPrefs.selected[monitorId]) {
        delete userPrefs.selected[monitorId]
        if (send)
            window.chrome.webview.postMessage({ type: 'set-wallpaper', 'monitor-id': monitorId, 'url': '' });
        updateMonitorIndicators();
        updateCardDescription();
    }
}

// INITIALIZATION SCRIPT
document.addEventListener("DOMContentLoaded", () => {
    for (var range of document.getElementsByClassName("range-input")) updateRange(range);
    for (var fileInput of document.getElementsByClassName("file-input")) updateFileInput(fileInput);
    for (var img of document.getElementsByTagName("img")) img.draggable = false;

    window.chrome.webview.postMessage({ type: 'request-prefs' });
    window.chrome.webview.postMessage({ type: 'request-monitor-ids' });
    window.chrome.webview.postMessage({ type: 'request-wallpaper-data' });

    populateExplore();
    setInterval(handleScrollShadows, 100);
    window.onresize();
});

// STYLING
function updateResponsiveElements() {
    // var computed = window.getComputedStyle(document.querySelector(".card"));
    // const getCSSVariable = (prop) => parseInt(window.getComputedStyle(document.body).getPropertyValue(prop));
    // // var cardWidth = parseInt(computed.getPropertyValue("width")) + parseInt(computed.getPropertyValue("margin-left")) + parseInt(computed.getPropertyValue("margin-right"));
    // // var totalSpace = document.querySelector(".content").clientWidth;
    // // var scrollboxWidth = totalSpace - getCSSVariable("--default-sidebar-width");
    // // var numberOfCards = Math.floor(scrollboxWidth/cardWidth);
    // // var newSidebarWidth = totalSpace - numberOfCards*cardWidth - 10;
    // // document.documentElement.style.setProperty("--sidebar-width", newSidebarWidth + "px");

    // var scrollboxWidth = document.querySelector(".content").clientWidth - getCSSVariable("--sidebar-width");
    // var cardWidth = getCSSVariable("--default-card-width") + parseInt(computed.getPropertyValue("margin-left")) + parseInt(computed.getPropertyValue("margin-right"));
    // document.documentElement.style.setProperty("--card-width", scrollboxWidth / Math.floor(scrollboxWidth / cardWidth) - 20 + "px");
    // for (var shadow of document.getElementsByClassName("scroll-shadow")) shadow.style.width = scrollboxWidth - 10 + "px";
}

// window.onresize = updateResponsiveElements;

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
    const nameMap = { "explore": "Explore", "modules": "Library", "develop": "Create", "settings": "Preferences" }
    setTimeout(() => title.textContent = nameMap[name], 100);
    setTimeout(() => title.style.opacity = 1, 100);

    if (activeTab == 'explore') {
        document.querySelector('.search').classList.add('active')
    }
    else document.querySelector('.search').classList.remove('active')

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
                console.log(previewModData, focusedId)
                if (previewModData && focusedId == previewModData.folderPath) {
                    unappendModData(previewModData);
                    previewModData = null;
                    updateMods();
                    return;
                }
                const modData = userPrefs.modData[focusedId];
                window.chrome.webview.postMessage({ type: "remove-wallpaper", folderPath: modData.folderPath });
                for (var id of Object.keys(exploreMods)) {
                    if (exploreMods[id] == focusedId) {
                        exploreMods[focusedIds.explore].installed = false;
                        document.getElementById(id).classList.remove("installed");
                    }
                }
                delete userPrefs.modOptions[focusedId]
                dumpUserPrefs();
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
            if (userPrefs.selected && userPrefs.selected[monitorId] == id) {
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
    // updateResponsiveElements();
    updateMonitorIndicators();
}

function updateMonitorIndicators() {
    if (!userPrefs.modData)
        return;
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
                div.setAttribute('onclick', `unsetByMonitorId(${JSON.stringify(monitorId)})`);
                indicators.appendChild(div);
            }
            var allMonitors = true;
            for (const monitorId of monitorIds) {
                if (userPrefs.selected && userPrefs.selected[monitorId] != id)
                    allMonitors = false;
            }
            if (allMonitors && monitorIds.length == 1) addIndicator(monitorIds[0]);
            else {
                for (const monitorId of monitorIds) {
                    if (userPrefs.selected && userPrefs.selected[monitorId] == id) addIndicator(monitorId);
                }
            }
        }
    }
}

function configureSetAsWallpaperButton() {
    const button = document.getElementById('set-wallpaper');
    const monitorArrow = button.querySelector(".monitor-arrow");
    const focusedId = focusedIds.modules
    if (!focusedId)
        return;
    var buttonDisabled = true;
    for (const monitorId of monitorIds) {
        if (userPrefs.selected[monitorId] != focusedId) {
            buttonDisabled = false;
            break;
        }
    }
    if (!monitorArrow)
        return;
    if (monitorIds.length > 1) {
        monitorArrow.classList.add('active');
    }
    else {
        monitorArrow.classList.remove('active');
    }

    const useUnset = monitorIds.length == 1;
    if (useUnset) {
        if (buttonDisabled) {
            console.log('unsetting');
            button.querySelector('.set-wallpaper-text').innerText = 'Unset as Wallpaper';
            button.classList.add("unset");
            button.onclick = () => {
                for (const monitorId of monitorIds) {
                    if (userPrefs.selected[monitorId] == focusedId) {
                        unsetByMonitorId(monitorId);
                    }
                }
            }
        }
        else {
            console.log('setting');
            button.querySelector('.set-wallpaper-text').innerText = 'Set as Wallpaper'
            button.classList.remove("unset");
            button.setAttribute("onclick", "selectFocusedCard()");
        }
    }
    else {
        // disable behavior
        if (buttonDisabled) {
            button.classList.add("inactive");
            button.disabled = true;
        }
        else {
            button.classList.remove("inactive");
            button.disabled = false;
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
            configureSetAsWallpaperButton();
            updateDevelopDescription();
        }
    }
}

function handleExitMonitorSelector(e) {
    // if (e.target.id != 'monitor-selector' && !e.target.classList.contains('monitor-item')) {
    document.getElementById('monitor-selector').classList.remove('active');
    // }
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
    const sortedMonitorIds = monitorIds.sort((a, b) =>
        (userPrefs.selected[a] === focusedIds.modules) - (userPrefs.selected[b] === focusedIds.modules)
    );
    for (const monitorId of sortedMonitorIds) {
        const div = document.createElement('div');
        div.classList.add('monitor-item');
        div.id = monitorId;
        if (userPrefs.selected[monitorId] == focusedIds.modules) {
            // div.classList.add('inactive');
            div.classList.add('unset');
            div.innerText = "Unset for " + cleanMonitorId(monitorId);
            div.setAttribute('onclick', `unsetByMonitorId(${JSON.stringify(monitorId)})`);
        }
        else {
            div.innerText = "Set for " + cleanMonitorId(monitorId);
            div.setAttribute("onclick", "selectMonitor(event, this)");
        }
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

    const button = cardDescription.querySelector("button");
    if (prefix == "modules") {
        configureSetAsWallpaperButton();
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
        var buttonDisabled = true;
        if (downloadingCards.includes(focusedIds[prefix])) {
            buttonDisabled = true;
            cardDescription.querySelector("button").innerText = "Downloading...";
            card.classList.add('loading')
        }
        else if (installed === false) {
            buttonDisabled = false;
            cardDescription.querySelector("button").innerText = "Add to library";
            card.classList.remove('loading')
        }
        else {
            buttonDisabled = true;
            cardDescription.querySelector("button").innerText = "Added to library";
            card.classList.remove('loading')
        }
        button.disabled = buttonDisabled;
        if (buttonDisabled) button.classList.add("inactive");
        else button.classList.remove("inactive");
    }
}

// CARD OPTIONS
function createInput(options, id, cardOptions = null) {
    console.log('creating input from ', options)
    if (!cardOptions)
        cardOptions = document.getElementById("card-options");
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
        if (options.value && options.type != "select") {
            input.setAttribute("value", options.value);
        }
        getter = () => input.value;
        if (type == "checkbox") {
            if (options.value) input.checked = options.value;
            getter = () => input.checked;
        }
        else if (type == "file") {
            getter = () => input.files[0];
            updateFileInput(input);
        }
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
            if (options.value) select.value = options.value
            getter = () => select.options[select.selectedIndex].text;
        }
        else if (type == "range" || type == "number") {
            console.log('creating range or number')
            if (options.min) input.setAttribute("min", options.min);
            if (options.max) input.setAttribute("max", options.max);
            if (options.step) input.setAttribute("step", options.step);
            if (options.value) {
                console.log('value is ', options.value.toString())
                input.setAttribute('value', options.value.toString())
                input.value = options.value.toString();
                console.log('input.value is', input.value)
            }
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
    console.log('updating to input.value', input.value)
    p.innerText = input.value;
}

function updateFileInput(el) {
    const p = el.parentNode.querySelector('p');
    console.log('file has value', el.value)
    if (el.files[0])
        p.style.setProperty('--after-content', `'${el.files[0].name}'`);
    else
        p.style.setProperty('--after-content', `''`);
}

function setCardOptions(json, cardOptions = null) {
    const settingForDevelop = cardOptions != null;
    if (!cardOptions)
        cardOptions = document.getElementById("card-options");
    cardOptions.innerHTML = "";
    inputGetters = {};
    var candidates = [];
    for (var id of Object.keys(json)) {
        const newInput = createInput(json[id], id, cardOptions);
        candidates.push(newInput);
    }
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

        if (!settingForDevelop) {
            const restorePrefs = document.createElement('a')
            restorePrefs.setAttribute('id', 'restore-preferences')
            restorePrefs.onclick = restoreModOptions
            restorePrefs.innerText = 'Restore default mod options'
            cardOptions.appendChild(restorePrefs);
        }
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
    if (activeTab != "modules")
        return;
    const id = focusedIds.modules;
    if (id) {
        const modData = userPrefs.modData[id];
        if (modData) {
            console.log('a')
            for (const inputId of Object.keys(inputGetters)) {
                console.log('b', inputId)
                const value = getInput(inputId);
                const options = userPrefs.modOptions[id]
                if (options.hasOwnProperty(inputId)) {
                    console.log('c')
                    const input = options[inputId];
                    if (input.value != value) {
                        input.value = value;
                        for (const monitorId of Object.keys(userPrefs.selected)) {
                            if (userPrefs.selected[monitorId] == id) {
                                const payload = { 'type': 'event', 'eventType': 'options-change', 'data': { 'id': inputId, 'value': value } };
                                console.log('payload', payload, 'to', monitorId)
                                window.chrome.webview.postMessage({
                                    type: 'send-to-wallpaper',
                                    'monitor-id': monitorId,
                                    data: payload
                                });

                                if (input.hasOwnProperty('onchange')) {
                                    if (options[inputId].hasOwnProperty('onchange')) {
                                        window.chrome.webview.postMessage({
                                            type: 'exec',
                                            'monitor-id': monitorId,
                                            data: `{ const f = ${options[inputId].onchange}; f("${value}"); };`
                                        });
                                    }
                                }
                                if (input.hasOwnProperty('onchangeload')) {
                                    if (options[inputId].hasOwnProperty('onchangeload')) {
                                        window.chrome.webview.postMessage({
                                            type: 'exec',
                                            'monitor-id': monitorId,
                                            data: `{ const f = ${options[inputId].onchangeload}; f("${value}"); };`
                                        });
                                    }
                                }
                            }
                        }
                    }
                }
            }
            dumpUserPrefs();
        }
    }
}

function sendWallpaperAllOptions(monitorId) {
    console.log('sending all options')
    const id = userPrefs.selected[monitorId];
    if (!id)
        return;
    const options = userPrefs.modOptions[id];
    if (!options)
        return;
    for (const inputId of Object.keys(options)) {
        const value = options[inputId].value
        const payload = { 'type': 'event', 'eventType': 'options-load', 'data': { 'id': inputId, 'value': value } };
        window.chrome.webview.postMessage({
            type: 'send-to-wallpaper',
            'monitor-id': monitorId,
            data: payload
        });

        if (options[inputId].hasOwnProperty('onload')) {
            window.chrome.webview.postMessage({
                type: 'exec',
                'monitor-id': monitorId,
                data: `{ const f = ${options[inputId].onload}; f("${value}"); };`
            });
        }
        if (options[inputId].hasOwnProperty('onchangeload')) {
            window.chrome.webview.postMessage({
                type: 'exec',
                'monitor-id': monitorId,
                data: `{ const f = ${options[inputId].onchangeload}; f("${value}"); };`
            });
        }
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
    if (previewModData)
        var filteredPrefs = {
            ...userPrefs,
            modOptions: Object.fromEntries(
                Object.entries(userPrefs.modOptions)
                    .filter(([key]) => key !== previewModData.folderPath)
            ),
            modData: Object.fromEntries(
                Object.entries(userPrefs.modData)
                    .filter(([key]) => key !== previewModData.folderPath)
            )
        };
    else
        var filteredPrefs = userPrefs;
    window.chrome.webview.postMessage({ type: 'dump-prefs', value: filteredPrefs });
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
    const restart = el.hasAttribute('requires-restart');
    if (restart) {
        modalDialog("Requires reload", "Would you like to restart the app now? If not, the change will apply the next time you close Octos.");
        modalListener = (state) => {
            if (state) {
                window.chrome.webview.postMessage({ type: 'restart' });
            }
        }
    }
}

function handleRecieveUserPrefs(msg) {
    userPrefs = Object.assign({
        modData: {},
        modOptions: {},
        selected: {},
        appOptions: appOptionsDefaults
    }, msg.data ?? {});
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

    // initDevelop();
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
    const card = document.getElementById(id);
    card.classList.add("installed");
    card.classList.remove('loading')
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
    if (!userPrefs.modData)
        return;
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

function containsAllChars(haystack, needle) {
    haystack = haystack.toLowerCase();
    needle = needle.toLowerCase();
    for (const char of needle) {
        if (!haystack.includes(char)) return false;
    }
    return true;
}

function onSearch(input) {
    const searchClear = input.parentNode.querySelector(".search-clear");
    searchClear.style.display = input.value != "" ? "block" : "none"
    filterExplore((data) => {
        const inputVal = input.value.toLowerCase();
        const inName = containsAllChars(data.name, input.value)
        const inKeywords = data.keywords?.some(keyword =>
            keyword.toLowerCase().includes(inputVal)
        );
        const inAuthor = containsAllChars(data.author, input.value)
        return inName || inKeywords || inAuthor
    })
}

function filterExplore(filterFn) {
    // const exploreScrollbox = document.getElementById("explore-scrollbox");
    for (const id of Object.keys(exploreMods)) {
        const card = document.getElementById(id);
        const modData = exploreMods[id];
        if (filterFn(modData)) {
            card.style.display = 'block'
        }
        else {
            card.style.display = 'none'
        }
    }
}

// DEVELOP
var requestingDevelopSession = false;

function onNewDevelopSession() {
    selectDevelopMod();
    initEditor();
}

function selectDevelopFolder() {
    window.chrome.webview.postMessage({ type: 'select-folder' });
    requestingDevelopSession = true;
}

function openDevelopFolder() {
    const modData = userPrefs.developMod;
    if (modData) {
        window.chrome.webview.postMessage({ type: 'open-folder', path: modData.folderPath });
        requestingDevelopSession = true;
    }
}

function selectDevelopMod() {
    const button = document.getElementById("set-develop-wallpaper");
    const modData = userPrefs.developMod;
    if (button.classList.contains('unset')) {
        for (const monitorId of monitorIds) {
            if (userPrefs.selected[monitorId] == modData.folderPath) {
                window.chrome.webview.postMessage({ type: 'set-wallpaper', 'monitor-id': monitorId, 'url': '' });
                delete userPrefs.selected[monitorId];
            }
            dumpUserPrefs();
            configureSetAsWallpaperButton();
            updateMonitorIndicators();
            updateDevelopDescription();
        }
        console.log('removing unset');
        button.classList.remove('unset');
        button.innerText = 'Set as Wallpaper';
    }
    else {
        if (modData) {
            for (const monitorId of monitorIds) {
                userPrefs.selected[monitorId] = modData.folderPath;
                window.chrome.webview.postMessage({ type: 'set-wallpaper', 'monitor-id': monitorId, 'url': modData.entryPath });
            }
            dumpUserPrefs();
            configureSetAsWallpaperButton();
            updateMonitorIndicators();
            updateDevelopDescription();
        }
        button.classList.add('unset');
        button.innerText = 'Unset Wallpaper';
    }
}

function developNew() {
    window.chrome.webview.postMessage({ type: 'create-wallpaper' });
}

function initDevelop() {
    if (!userPrefs.developMod) {
        document.getElementById('develop-tab').classList.remove('developing');
        // window.chrome.webview.postMessage({ type: 'select-folder' });
    }
    else {
        const folderPath = userPrefs.developMod.folderPath;
        if (folderPath)
            window.chrome.webview.postMessage({ type: 'request-config', folderPath });
    }
}

function exitDevelop() {
    console.log('exiting develop');
    delete userPrefs.developMod;
    document.getElementById('develop-tab').classList.remove('developing');
    dumpUserPrefs();
    if (developDocsFrame) {
        developDocsFrame.remove();
        developDocsFrame = '';
    }
}

function dumpDevelopConfig() {
    const modData = userPrefs.developMod;
    if (modData) {
        console.log(modData.configPath);
        window.chrome.webview.postMessage({ type: 'dump-config', configPath: modData.configPath, data: modData.config });
    }
}

function handleConfigData(modData) {
    console.log('Received config data ', modData);
    userPrefs.developMod = modData;
    document.getElementById('develop-tab').classList.add('developing');
    updateDevelopDescription();
    configureSetAsWallpaperButton();
    console.log('dumping prefs', userPrefs)
    dumpUserPrefs();
    if (requestingDevelopSession) {
        onNewDevelopSession();
        requestingDevelopSession = false;
    }
    updateEditorContent();
}

function updateEditorContent() {
    if (userPrefs.developMod && window.setEditorValue)
        window.setEditorValue(JSON.stringify(userPrefs.developMod.config, null, 4));
}

function updateDevelopDescription() {
    const modData = userPrefs.developMod;
    if (!modData)
        return;
    const developDescription = document.getElementById('develop-card-description');
    const optionsLabel = developDescription.querySelector('.develop-options-label-content')
    if (modData.config && modData.config.options) {
        setCardOptions(modData.config.options, document.getElementById('develop-options'));
        optionsLabel.innerText = "Mod options";
    }
    else if (modData.options) {
        setCardOptions(modData.config.options, document.getElementById('develop-options'));
        optionsLabel.innerText = "Mod options";
    }
    developDescription.querySelector('.description-content').innerText = modData.config.description || modData.description || "Add a description.";
    developDescription.querySelector('.author-content').innerText = modData.config.author || modData.author || "Author";
    developDescription.querySelector('.title-content').innerText = modData.config.name || modData.name || "Mod Name";
    const button = developDescription.querySelector('.set-wallpaper');
    var buttonDisabled = true;
    for (const monitorId of monitorIds) {
        if (userPrefs.selected[monitorId] != modData.folderPath) {
            buttonDisabled = false;
            break;
        }
    }
    if (buttonDisabled) {
        button.classList.add('unset');
        button.innerText = 'Unset Wallpaper';
    }
    else {
        button.classList.remove('unset');
        button.innerText = 'Set as Wallpaper';
    }
}

function editThis(el) {
    const content = el.parentNode.querySelector('.edit-content');
    if (content) {
        const field = content.getAttribute('field');
        if (field) {
            modalDialog('Edit ' + field, '', content.innerText);
            modalListener = (state, text) => {
                if (state && text != "") {
                    userPrefs.developMod.config[field] = text;
                    userPrefs.developMod[field] = text;
                    console.log(userPrefs.developMod.config);
                    content.innerText = text;
                    dumpDevelopConfig();
                    updateEditorContent();
                }
            }
        }
    }
}

function editDevelopOptions() {
    const modData = userPrefs.developMod;
    if (modData && modData.configPath)
        window.chrome.webview.postMessage({ type: 'open-file', path: modData.configPath });
}

var developTab = 'files';
var developDocsFrame;

function onTabLoad(tab, tabContent) {
    console.log('loading', tab, tabContent)
    switch (tab) {
        case 'docs':
            if (!developDocsFrame) {
                developDocsFrame = document.createElement('iframe');
                developDocsFrame.src = 'https://underpig1.github.io/octos/docs';
                tabContent.appendChild(developDocsFrame);
            }
            break;
        default:
            break;
    }
}

function onTabUnload(tab, tabContent) {
    console.log('unloading', tab, tabContent)
    switch (tab) {
        case 'docs':
            // setTimeout(() => {
            //     if (developDocsFrame) {
            //         tabContent.removeChild(developDocsFrame);
            //         developDocsFrame = '';
            //     }
            // }, 500);
            break;
        default:
            break;
    }
}

function setDevelopTab(el, tab) {
    if (developTab == tab)
        return;
    for (const tabContent of document.getElementsByClassName('develop-tab-content')) {
        const tabId = tabContent.id.split('-')[0];
        if (tabId == tab) {
            tabContent.classList.add('active');
            onTabLoad(tab, tabContent);
        }
        else {
            if (developTab == tabId) {
                onTabUnload(tabId, tabContent);
                tabContent.classList.remove('active');
            }
        }
    }
    for (const tab of document.getElementsByClassName('develop-tab')) {
        if (tab == el) {
            tab.classList.add('active');
        }
        else tab.classList.remove('active');
    }
    developTab = tab;
}

function openDevelopDevTools() {
    if (userPrefs.developMod) {
        for (const monitorId of Object.keys(userPrefs.selected)) {
            if (userPrefs.selected[monitorId] == userPrefs.developMod.folderPath) {
                console.log('opening devtools');
                window.chrome.webview.postMessage({ type: 'open-wallpaper-devtools', 'monitor-id': monitorId });
            }
        }
    }
}

function initEditor() {
    require.config({ paths: { 'vs': 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.38.0/min/vs' } });
    require(['vs/editor/editor.main'], () => {
        const container = document.getElementById('develop-editor');
        const editor = monaco.editor.create(container, {
            value: '// Start coding...',
            language: 'json',
            theme: 'vs-dark',
        });

        if (userPrefs.developMod) {
            updateEditorContent();
        }

        window.getEditorValue = () => editor.getValue();
        window.setEditorValue = (text) => editor.setValue(text);
        editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
            if (userPrefs.developMod) {
                const content = JSON.parse(editor.getValue());
                userPrefs.developMod.config = content;
                dumpDevelopConfig();
                updateDevelopDescription();
            }
        });

        const resizeObserver = new ResizeObserver(() => {
            editor.layout();
        });
        resizeObserver.observe(container);
    });
}

// PREVIEW
let previewModData;

function appendModData(config) {
    const modId = config.folderPath;
    if (modId) {
        userPrefs.modData[config.folderPath] = config;
        if (config.options) {
            userPrefs.modOptions[config.folderPath] = JSON.parse(JSON.stringify(config.options));
        }
    }
}

function unappendModData(config) {
    const modId = config.folderPath;
    if (modId) {
        delete userPrefs.modData[config.folderPath];
        delete userPrefs.modOptions[config.folderPath];
    }
}

function handleOnPreview(config) {
    console.log('got preview', config)
    if (!config)
        return;
    for (const monitorId of Object.keys(userPrefs.selected)) {
        unsetByMonitorId(monitorId, false);
    }
    if (config.folderPath) {
        if (previewModData) {
            unappendModData(previewModData);
        }
        previewModData = config;
        appendModData(config);
        for (const monitorId of monitorIds) {
            userPrefs.selected[monitorId] = config.folderPath;
        }
        updateMods();
    }
}