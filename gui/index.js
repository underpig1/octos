var focusedIDs = { explore: null, modules: null };
var selectedID;
var isVisible = true;
var inputGetters = {};
var installedMods = {};
var exploreMods = {};
var activeTab = "explore";
var modalListener = () => null;
var downloadingCards = [];
var develop = {}

window.link.getVisibility().then((state) => {
    isVisible = state;
    updateVisibilityIcon();
});

document.addEventListener("DOMContentLoaded", () => {
    for (var range of document.getElementsByClassName("range-input")) updateRange(range);
    for (var img of document.getElementsByTagName("img")) img.draggable = false;
});

function developNew() {
    window.link.newMod().then((dir) => {
        develop.dir = dir;
        develop.name = dir.split(/\\|\//g).pop();
        develop.description = "This is a very cool mod you made."
        enableDeveloping();
        updateDevelopModInfo();
    }).catch(() => {});
}

function developFile() {
    window.link.openMod().then((dir) => {
        develop.dir = dir;
        window.link.developConfig.get(dir).then((config) => {
            if (config) {
                develop.name = config.name;
                develop.description = config.description;
                develop.events = config.options ? config.options.events : {};
            }
            enableDeveloping();
            updateDevelopModInfo();
        });
    }).catch(() => {});
}

function renameMod(name) {
    modalDialog("Rename mod", "", develop.name ? develop.name : "Mod name");
    modalListener = (state, text) => {
        if (state) {
            if (develop.dir) {
                develop.name = text;
                window.link.renameMod(develop.dir, text).then((dir) => develop.dir = dir);
                updateDevelopModInfo();
            }
        }
    }
}

function editConfig(listener) {
    window.link.developConfig.get(develop.dir).then((config) => window.link.developConfig.set(develop.dir, listener(config)));
}

function editModDescription(name) {
    modalDialog("Edit description", "", develop.description ? develop.description : "Mod description");
    modalListener = (state, text) => {
        if (state) {
            develop.description = text;
            updateDevelopModInfo();
        }
    }
}

function openModFolder() {
    window.link.openModFolder(develop.dir);
}

function buildMod() {
    window.link.buildMod(develop.dir);
}

function updateDevelopModInfo() {
    if (develop) {
        document.getElementById("develop-name").innerText = develop.name;
        document.getElementById("develop-description").innerText = develop.description;
    }
    develop.events = {
        mouse: document.getElementById("mouse-checkbox").checked,
        keyboard: document.getElementById("keyboard-checkbox").checked,
        media: document.getElementById("media-checkbox").checked,
    }
    editConfig((config) => {
        config.name = develop.name;
        config.description = develop.description;
        if (!config.options) config.options = {};
        config.options.events = develop.events;
        return config;
    });
    updateWorking();
}

function playMod() {
    if (develop) window.link.runMod(develop.dir);
}

function stopMod() {
    window.link.stopMod();
}

function debugMod() {
    window.link.toggleDebug();
}

function exit() {
    window.link.close();
}

function minimize() {
    window.link.minimize();
}

function fullscreen() {
    window.link.fullscreen();
}

function update() {
    handleScrollShadows();
}

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

setInterval(update, 100);

function updateRange(el) {
    var input = el;
    var p = el.parentNode.querySelector(".range-value");
    p.innerText = input.value;
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
    setTimeout(() => title.textContent = name[0].toUpperCase() + name.slice(1), 100);
    setTimeout(() => title.style.opacity = 1, 200);

    var highlight = document.querySelector(".nav-highlight");
    highlight.style.setProperty("--nav-active", Array.prototype.indexOf.call(el.parentNode.children, el) - 1);
    cleanFocus();
    updateCardDescription();
}

function refresh() {
    window.link.refresh();
}

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

function updateCardDescription() {
    cleanFocus();
    var cardData = getFocusedCardData();
    if (cardData) setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, cardData.options);
}

function cleanFocus() {
    const cardDescription = document.getElementById(activeTab + "-card-description");
    var activeIndex = activeTab == "modules" ? installedMods : exploreMods;
    if (Object.keys(activeIndex).length == 0) {
        cardDescription.classList.add("inactive");
    }
    else {
        cardDescription.classList.remove("inactive");
        if (focusedIDs[activeTab] == null || !document.getElementById(focusedIDs[activeTab]).classList.contains("focused")) focusCard(document.getElementById(Object.keys(activeIndex)[0]));
    }
}

function setCardDescription(prefix = "explore", title = "", author = "", description = "", options = null) {
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
    if (selectedID == focusedIDs.modules && prefix == "modules") cardDescription.querySelector("button").classList.add("inactive");
    else cardDescription.querySelector("button").classList.remove("inactive");
    if (prefix == "explore") {
        if (downloadingCards.includes(focusedIDs[prefix])) {
            cardDescription.querySelector("button").classList.add("inactive");
            cardDescription.querySelector("button").innerText = "Downloading...";
        }
        else {
            cardDescription.querySelector("button").classList.remove("inactive");
            cardDescription.querySelector("button").innerText = "Download";
        }
    }
}

function updateVisibilityIcon() {
    var visibleIcon = document.getElementById("visible-icon");
    if (isVisible) visibleIcon.src = "img/visibility.png";
    else visibleIcon.src = "img/invisible.png";
}

function toggleVisibility() {
    isVisible = !isVisible;
    updateVisibilityIcon();
    window.link.setVisibility(isVisible);
}

function enableDeveloping() {
    var tab = document.getElementById("develop-tab");
    tab.classList.add("developing");
}

function disableDeveloping() {
    var tab = document.getElementById("develop-tab");
    tab.classList.remove("developing");
}

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
        else if (type == "range") {
            if (options.min) input.setAttribute("min", options.min);
            if (options.max) input.setAttribute("max", options.max);
            if (options.step) input.setAttribute("step", options.step);
            updateRange(input);
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

function setCardOptions(json) {
    const cardOptions = document.getElementById("card-options");
    cardOptions.innerHTML = "";
    inputGetters = {};
    for (var id of Object.keys(json)) {
        createInput(json[id], id);
    }
}

function restorePreferences() {
    modalDialog("Restore preferences", "Are you sure you want to restore default mod preferences? This action is irreversible.");
    modalListener = (state) => {
        if (focusedIDs.modules && state) {
            var focusedCard = getFocusedCardData();
            window.link.getPrefs().then((prefs) => {
                if (prefs.prefs) {
                    var options = prefs.prefs[focusedCard.name];
                    if (options) {
                        var defaults = options.defaults;
                        focusedCard.options = defaults;
                        window.link.restoreModPrefs();
                        updateCardDescription();
                    }
                }
            });
        }
    }
}

function createCard(id = "", title = "Mod name", author = null, backgroundImage = "") {
    var template = document.getElementById("card-template");
    var div = template.content.cloneNode(true).firstElementChild;
    div.querySelector(".card-title").innerText = title;
    div.querySelector(".card-author").innerText = author ? "By " + author : "";
    div.id = id;
    if (backgroundImage) div.style.backgroundImage = `url('${backgroundImage}')`;
    return div;
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

function updateMods() {
    window.link.getPrefs().then((prefs) => {
        const modScrollbox = document.getElementById("modules-scrollbox");
        var tempFocus = Object.keys(installedMods).length > 0 ? focusedIDs.modules ? installedMods[focusedIDs.modules].name : null : null;
        installedMods = {};
        modScrollbox.innerHTML = "";
        var modNames = prefs.mods.map((x) => x.name);
        for (var id in modNames) {
            var name = modNames[id];
            var img = prefs.images ? prefs.images[name] : null;
            var config = prefs.configs[name];
            var author = config ? config.author : null;
            var modPrefs = prefs.prefs ? prefs.prefs[name] ? prefs.prefs[name].local : null : null;
            var card = createCard(id, name, author, img);
            if ((!tempFocus && id == 0) || tempFocus == name) {
                for (var child of modScrollbox.children) child.classList.remove("focused");
                card.classList.add("focused");
                focusedIDs.modules = id;
            }
            if (prefs.selected == name) {
                for (var child of modScrollbox.children) child.classList.remove("selected");
                card.classList.add("selected");
                selectedID = id;
            }
            modScrollbox.appendChild(card);
            installedMods[id] = { name, img, author, el: card, config, options: modPrefs };
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
        populateExplore();
    });
}

function selectFocusedCard() {
    const modScrollbox = document.getElementById("modules-scrollbox");
    var focusedCard = document.getElementById(focusedIDs.modules);
    for (var child of modScrollbox.children) child.classList.remove("selected");
    focusedCard.classList.add("selected");
    selectedID = focusedCard.id;
    var name = installedMods[selectedID].name;
    window.link.selectMod(name);
}

function removeFocusedCard() {
    modalListener = (state) => {
        if (state) window.link.removeMod(getFocusedCardData().name);
    }
    modalDialog("Remove mod", "Are you sure you want to remove this mod?");
}

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

function uploadMod() {
    window.link.upload();
}

function downloadFocusedCard() {
    var id = focusedIDs.explore;
    var name = exploreMods[id].name;
    downloadingCards.push(id);
    updateCardDescription();
    window.link.downloadMod(name).then(() => {
        downloadingCards.splice(downloadingCards.indexOf(id), 1);
        updateCardDescription();
    }).catch(() => {
        downloadingCards.splice(downloadingCards.indexOf(id), 1);
        updateCardDescription();
        modalListener = (state) => {
            if (state) downloadFocusedCard();
        }
        modalDialog("Download failed", "There may be something wrong with your Internet. Try again?");
    });
}

function goToSource() {
    var id = focusedIDs.explore;
    var name = exploreMods[id].name;
    window.link.goToSource(name);
}

function restoreUserPrefs() {
    modalDialog("Restore settings", "Are you sure you want to restore default settings?");
    modalListener = (state) => {
        if (state) {
            window.link.userPrefs.restore();
            retrieveUserPrefs();
        }
    }
}

function updateUserPrefs(el) {
    var field = el.id;
    if (el.type == "checkbox") var value = el.checked;
    else var value = el.value;
    window.link.userPrefs.set(field, value);
}

function retrieveUserPrefs() {
    var inputs = document.getElementsByClassName("settings-input");
    const pass = (el) => window.link.userPrefs.get(el.id).then((value) => {
        if (value != null) {
            if (el.type == "checkbox") el.checked = value;
            else el.value = value;
        }
    });
    for (var el of inputs) pass(el);
}

function updateWorking() {
    window.link.userPrefs.set("working", develop);
}

function retrieveWorking() {
    window.link.userPrefs.get("working").then((content) => {
        if (content) {
            develop = content;
            enableDeveloping();
            updateDevelopModInfo();
        }
    });
}

retrieveUserPrefs();
populateExplore();
retrieveWorking()

function populateExplore() {
    window.link.request.modData().then((data) => {
        const exploreScrollbox = document.getElementById("explore-scrollbox");
        exploreScrollbox.innerHTML = "";
        exploreMods = {};
        for (var id in Object.keys(data)) {
            var name = Object.keys(data)[id];
            var modData = data[name];
            var valid = true;
            for (var card of Object.values(installedMods)) valid = !valid || !(card.name == name && card.description == modData.description && card.author == modData.author);
            if (valid) {
                if (modData.image) {
                    window.link.request.modImage(name).then((data) => {
                        var card = createCard("explore-" + id, name, modData.author, data);
                        exploreScrollbox.appendChild(card);
                        exploreMods["explore-" + id] = { name, author: modData.author, description: modData.description }
                        updateCardDescription();
                    });
                }
                else {
                    var card = createCard("explore-" + id, name, modData.author, null);
                    exploreScrollbox.appendChild(card);
                    exploreMods["explore-" + id] = { name, author: modData.author, description: modData.description }
                    updateCardDescription();
                }
            }
        }
    });
}