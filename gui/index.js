var focusedCard;
var isVisible = true;
var developing = false;
var inputGetters = {};

document.addEventListener("DOMContentLoaded", () => {
    for (var range of document.getElementsByClassName("range-input")) {
        range.querySelector(".range-value").innerText = range.querySelector("input").value;
    }
});

function developNew() {
    enableDeveloping();
    // more
}

function update() {
    handleScrollShadows();
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
}

function toggleVisibility() {
    isVisible = !isVisible;
    var visibleIcon = document.getElementById("visible-icon");
    if (isVisible) visibleIcon.src = "img/visibility.png";
    else visibleIcon.src = "img/invisible.png";
    // more
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
        var div = template.content.cloneNode(true);
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