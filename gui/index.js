var focusedCards = { explore: null, modules: null };
var selectedID;
var isVisible = true;
var developing = false;
var inputGetters = {};
var installedMods = {};
var activeTab = document.querySelector(".tab-content.active");

window.link.getVisibility().then((state) => {
    isVisible = state;
    updateVisibilityIcon();
});

document.addEventListener("DOMContentLoaded", () => {
    for (var range of document.getElementsByClassName("range-input")) {
        range.querySelector(".range-value").innerText = range.querySelector("input").value;
    }
});

function developNew() {
    enableDeveloping();
    // more
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
        if (Math.abs(content.scrollHeight - content.clientHeight - content.scrollTop) < 1) top.style.opacity = 1;
        else top.style.opacity = 0;
        if (content.scrollTop < 1) bottom.style.opacity = 1;
        else bottom.style.opacity = 0;

        if (content.clientHeight == content.scrollHeight) {
            top.style.opacity = 0;
            bottom.style.opacity = 0;
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
}

function focusCard(el) {
    for (var card of el.parentNode.getElementsByClassName("card")) card.classList.remove("focused");
    el.classList.add("focused");
    focusedCards[activeTab] = el;
    updateCardDescription();
}

function getFocusedCardData() {
    return focusedCards[activeTab] ? installedMods[focusedCards[activeTab].id] : null;
}

function updateCardDescription() {
    var cardData = getFocusedCardData();
    setCardDescription(activeTab, cardData.name, cardData.author, cardData.description, cardData.options);
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
    developing = true;
}

function createInput(options, id) {
    const cardOptions = document.getElementById("card-options");
    var type = options.type;
    var getter = () => null;
    if (!type) type = "checkbox";
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

function createCard(id = "", title = "Mod name", author = null, backgroundImage = "") {
    var template = document.getElementById("card-template");
    var div = template.content.cloneNode(true).firstElementChild;
    div.querySelector(".card-title").innerText = title;
    div.querySelector(".card-author").innerText = author ? "By " + author : "";
    div.id = id;
    if (backgroundImage) div.style.backgroundImage = `url('${backgroundImage}')`;
    return div;
}

function updateDevelopMetadata() {
    var mousePerms = document.getElementById("mouse-checkbox").checked;
    var keyboardPerms = document.getElementById("keyboard-checkbox").checked;
    var mediaPerms = document.getElementById("media-checkbox").checked;
    // more
}

function updateMods() {
    window.link.getPrefs().then((prefs) => {
        const modScrollbox = document.getElementById("modules-scrollbox");
        var tempFocus = Object.keys(installedMods).length > 0 ? installedMods[focusedCards.modules.id].name : null;
        installedMods = {};
        modScrollbox.innerHTML = "";
        var modNames = prefs.mods.map((x) => x.name);
        for (var id in modNames) {
            var name = modNames[id];
            var img = prefs.images ? prefs.images[name] : null;
            var config = prefs.configs[name];
            var author = config ? config.author : null;
            var modPrefs = prefs.prefs ? prefs.prefs[name] : null;
            var card = createCard(id, name, author, img);
            if ((!tempFocus && id == 0) || tempFocus == name) {
                for (var child of modScrollbox.children) child.classList.remove("focused");
                card.classList.add("focused");
                focusedCards.modules = card;
            }
            if (prefs.selected == name) {
                for (var child of modScrollbox.children) child.classList.remove("selected");
                card.classList.add("selected");
                selectedID = id;
            }
            modScrollbox.appendChild(card);
            installedMods[id] = { name, img, author, el: card, config, options: modPrefs };
        }
        var uploadCard = document.getElementById("card-upload-template").content.cloneNode(true).firstElementChild;
        modScrollbox.appendChild(uploadCard);
        updateCardDescription();
    });
}

function selectFocusedCard() {
    const modScrollbox = document.getElementById("modules-scrollbox");
    var focusedCard = focusedCards.modules;
    for (var child of modScrollbox.children) child.classList.remove("selected");
    focusedCard.classList.add("selected");
    selectedID = focusedCard.id;
    var name = installedMods[selectedID].name;
    window.link.selectMod(name);
}