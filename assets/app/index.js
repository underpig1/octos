// WINDOW ACTIONS
document.getElementById('topbar').addEventListener('mousedown', (e) => {
    if (e.target === e.currentTarget)
        window.chrome.webview.postMessage({ type: "drag" });
});

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
        handleRecievePrefs(msg);
});

// INITIALIZATION SCRIPT
document.addEventListener("DOMContentLoaded", () => {
    for (var range of document.getElementsByClassName("range-input")) updateRange(range);
    for (var img of document.getElementsByTagName("img")) img.draggable = false;

    window.chrome.webview.postMessage({ type: 'request-prefs' });
    window.chrome.webview.postMessage({ type: 'request-wallpaper-data' });
    setInterval(handleScrollShadows, 100);
});

// STYLING
window.onresize = () => {
    updateResponsiveElements();
}

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
    const nameMap = { "explore": "Explore", "modules": "Installed modules", "develop": "Developer menu", "settings": "Settings" }
    setTimeout(() => title.textContent = nameMap[name], 100);
    setTimeout(() => title.style.opacity = 1, 200);

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

function getFocusedCardData() {
    if (focusedIDs[activeTab]) {
        var id = focusedIDs[activeTab];
        if (activeTab == "modules") return installedMods[id];
        else if (activeTab == "explore") return exploreMods[id];
    }
    return null;
}

function cleanFocus() {
    const cardDescription = document.getElementById(activeTab + "-card-description");
    if (cardDescription) {
        var activeIndex = activeTab == "modules" ? installedMods : exploreMods;
        if (Object.keys(activeIndex).length == 0) {
            cardDescription.classList.add("inactive");
        }
        else {
            cardDescription.classList.remove("inactive");
            if (focusedIDs[activeTab] == null || !document.getElementById(focusedIDs[activeTab]).classList.contains("focused")) focusCard(document.getElementById(Object.keys(activeIndex)[0]));
        }
    }
}

function createCard(id = "", title = "Mod name", author = null, backgroundImage = "") {
    var template = document.getElementById("card-template");
    var div = template.content.cloneNode(true).firstElementChild;
    div.querySelector(".card-title").innerText = title;
    div.querySelector(".card-author").innerText = author ? "By " + author : "";
    div.id = id;
    if (backgroundImage) div.style.backgroundImage = `url('${backgroundImage.replace(/\\/g, "/")}')`;
    return div;
}

function removeFocusedCard() {
    modalListener = (state) => {
        if (state) {
            var folderPath = getFocusedCardData().folderPath;
            if (focusedIDs.modules == selectedID) {
                for (var id of Object.keys(installedMods)) {
                    if (id != selectedID) {
                        focusCard(document.getElementById(id));
                        selectFocusedCard();
                        break;
                    }
                }
            }
            window.chrome.webview.postMessage({type:"remove-wallpaper", folderPath});
            for (var id of Object.keys(exploreMods)) {
                if (exploreMods[id] == folderPath) {
                    exploreMods[focusedIDs.explore].installed = false;
                    document.getElementById(id).classList.remove("installed");
                }
            }
        }
    }
    modalDialog("Remove mod", "Are you sure you want to remove this mod?");
}

// CARD DESCRIPTION
function updateCardDescription() {
    cleanFocus();
    var cardData = getFocusedCardData();
    if (cardData) setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, cardData.options, cardData.installed);
}

function setCardDescription(prefix = "explore", title = "", author = "", description = "", options = null, installed = false) {
    const cardDescription = document.getElementById(prefix + "-card-description");
    cardDescription.querySelector(".title-content").innerText = title;
    if (author) cardDescription.querySelector(".author").innerHTML = `By <a class="author-content">${author}</a>`;
    else cardDescription.querySelector(".author").innerHTML = "";
    cardDescription.querySelector(".description").innerText = description;
    if (options) {
        setCardOptions(options);
        cardDescription.classList.remove("empty");
    }
    else cardDescription.classList.add("empty");

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
    for (var id of Object.keys(json)) {
        createInput(json[id], id);
    }
}

function restorePreferences() { // TBI
    modalDialog("Restore preferences", "Are you sure you want to restore default mod preferences? This action is irreversible.");
    modalListener = (state) => {
        if (focusedIDs.modules && state)
            if (prefs.value) {
                var focusedCard = getFocusedCardData();
                var options = prefs.value[focusedCard.name];
                if (options) {
                    var defaults = options.defaults;
                    focusedCard.options = defaults;
                    restoreModPrefs(focusedCard.name);
                    updateCardDescription();
                }
            }
    }
}

function updateCardOptions() {
    if (focusedIDs.modules) {
        var focusedCard = getFocusedCardData();
        var name = focusedCard.name;
        var modData = focusedCard.options;
        getInput();
        var options = {};
        for (var id of Object.keys(inputGetters)) {
            var value = getInput(id);
            modData[id].value = value;
            options[id] = getInput(id);
        }
        if (Object.keys(options).length > 0) window.link.setModPrefs(name, options);
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
var userPrefsDefaults = {
    prefs: "default"
}

function restoreUserPrefs() {
    modalDialog("Restore settings", "Are you sure you want to restore default settings?");
    modalListener = (state) => {
        if (state) {
            userPrefs = JSON.parse(JSON.stringify(userPrefsDefaults))
            window.chrome.webview.postMessage({ type: 'dump-prefs', value: userPrefs });
            retrieveUserPrefs();
        }
    }
}

function updateUserPrefs(el) {
    if (el.type == "checkbox") var value = el.checked;
    else var value = el.value;
    userPrefs[el.id] = value;
    window.chrome.webview.postMessage({ type: 'dump-prefs', value: userPrefs });
}

function handleRecievePrefs(msg) {
    userPrefs = msg.data
    var inputs = document.getElementsByClassName("settings-input");
    for (var el of inputs) {
        if (userPrefs[el.id] != null) {
            if (el.type == "checkbox") el.checked = userPrefs[el.id].value;
            else el.value = userPrefs[el.id].value;
        }
    }
}

var focusedIDs = { explore: null, modules: null };
var selectedID;
var isVisible = true;
var inputGetters = {};
var installedMods = {};
var exploreMods = {};
var activeTab = "modules";
var modalListener = () => null;
var downloadingCards = [];

// TO BE IMPLEMENTED
function handleRecieveModData(msg) {
    userPrefs.mods = msg.data;
}

function updateMods() {
    const modScrollbox = document.getElementById("modules-scrollbox");
    var tempFocus = Object.keys(installedMods).length > 0 ? focusedIDs.modules ? installedMods[focusedIDs.modules].name : null : null;
    installedMods = {};
    modScrollbox.innerHTML = "";
    for (var modPrefs in userPrefs.mods) {
        const id = modPrefs.folderPath
        const name = modPrefs.name
        var card = createCard(id, name, modPrefs.author, modPrefs.imagePath);
        if ((!tempFocus && id == 0) || tempFocus == name) {
            for (var child of modScrollbox.children) child.classList.remove("focused");
            card.classList.add("focused");
            focusedIDs.modules = id;
        }
        if (userPrefs.selected == name) {
            for (var child of modScrollbox.children) child.classList.remove("selected");
            card.classList.add("selected");
            selectedID = id;
        }
        modScrollbox.appendChild(card);
        installedMods[id] = {...modPrefs, el: card};
    }
    var focusCheck = false;
    for (var mod of modScrollbox.childNodes) focusCheck = focusCheck || mod.classList.contains("focused");
    if (!focusCheck) {
        var firstCard = modScrollbox.querySelector(".card");
        firstCard.classList.add("focused");
        focusedIDs.modules = firstCard.id;
    }
    var uploadCard = document.getElementById("card-upload-template").content.cloneNode(true).firstElementChild;
    modScrollbox.appendChild(uploadCard);
    updateCardDescription();
    updateDownloadedMods();
}

function selectFocusedCard() {
    const modScrollbox = document.getElementById("modules-scrollbox");
    var focusedCard = document.getElementById(focusedIDs.modules);
    for (var child of modScrollbox.children) child.classList.remove("selected");
    focusedCard.classList.add("selected");
    selectedID = focusedCard.id;
    var folderPath = installedMods[selectedID].folderPath;
    window.chrome.webview.postMessage({ type: 'select-wallpaper', folderPath });
}

// function uploadMod() {
//     window.link.upload();
// }

// function downloadFocusedCard() {
//     var id = focusedIDs.explore;
//     var name = exploreMods[id].name;
//     downloadingCards.push(id);
//     updateCardDescription();
//     window.link.downloadMod(name).then(() => {
//         downloadingCards.splice(downloadingCards.indexOf(id), 1);
//         exploreMods[id].installed = true;
//         document.getElementById(id).classList.add("installed");
//         updateCardDescription();
//         updateMods();
//     }).catch((err) => {
//         downloadingCards.splice(downloadingCards.indexOf(id), 1);
//         exploreMods[id].installed = true;
//         document.getElementById(id).classList.add("installed");
//         updateCardDescription();
//         updateMods();
//         modalListener = (state) => {
//             if (state) downloadFocusedCard();
//         }
//         modalDialog("Download failed", "There may be something wrong with your Internet. Try again?");
//     });
// }

// function goToSource() {
//     var id = focusedIDs.explore;
//     var name = exploreMods[id].name;
//     window.link.goToSource(name);
// }

// function updateWorking() {
//     window.link.userPrefs.set("working", develop);
// }

// function retrieveWorking() {
//     window.link.userPrefs.get("working").then((content) => {
//         if (content) {
//             develop = content;
//             enableDeveloping();
//             updateDevelopModInfo();
//         }
//     });
// }

// function updateDownloadedMods() {
//     for (var id of Object.keys(exploreMods)) {
//         var modData = exploreMods[id];
//         var installed = false;
//         for (var card of Object.values(installedMods)) {
//             if (card.name == modData.name && card.description == modData.description && card.author == modData.author) {
//                 installed = true;
//                 break;
//             }
//         }
//         modData.installed = installed;
//         var card = document.getElementById(id);
//         if (installed) card.classList.add("installed");
//         else card.classList.remove("installed");
//     }
// }

// document.addEventListener("DOMContentLoaded", () => {
//     retrieveUserPrefs();
//     populateExplore();
//     retrieveWorking();
// });

// function populateExplore() {
//     window.link.request.modData().then((data) => {
//         const exploreScrollbox = document.getElementById("explore-scrollbox");
//         exploreScrollbox.innerHTML = "";
//         exploreMods = {};
//         (async () => {
//             for (var id = 0, p = Promise.resolve(); id < Object.keys(data).length; id++) {
//                 var name = Object.keys(data)[id];
//                 var modData = data[name];
//                 await window.link.request.modImage(name).then((data) => {
//                     var card = createCard("explore-" + id, name, modData.author, data);
//                     exploreScrollbox.appendChild(card);
//                     exploreMods["explore-" + id] = { name, author: modData.author, description: modData.description }
//                 });
//             }
//             updateDownloadedMods();
//             cleanFocus();
//             updateCardDescription();
//         })();
//     });
// }