const fs = require("fs");
const path = require("path");

function replaceWidgets() {
    const widgets = document.querySelectorAll("widget");
    for (const widget of widgets) {
        var name = widget.getAttribute("name");
        widget.innerHTML = readWidget(name);

        var widgetStyles = window.getComputedStyle(widget);
        for (const style of widgetStyles) {
            var propName = style;
            var propValue = widgetStyles.getPropertyValue(style + "");

            widget.style.setProperty(propName, propValue);
        }
    }
}

function readWidget(name) {
    return fs.readFileSync(path.join(__dirname, "../..", "default-widget", name, "index.html"));
}

function injectHTMLByNameScript(name) {
    return `var widgets = document.querySelectorAll("widget[name='${name}']")
for (const widget of widgets) widget.innerHTML=\`${readWidget(name)}\``;
}

function setStylesByNameScript(name) {
    return `var widgets = document.querySelectorAll("widget[name='${name}']");
for (const widget of widgets) {
    /*var widgetStyles = window.getComputedStyle(widget);
    var propNames = [], propValues = [];
    for (const style of widgetStyles) {
        propNames.push("--" + style);
        propValues.push(widgetStyles.getPropertyValue(style + ""));
    }

    var content = "";
    for (var i = 0; i < propNames.length; i++) {
        content += propNames[i] + ": " + propValues[i] + " !important; "
    }
    widget.querySelector("style").sheet.insertRule(":root { " + content + " }");*/

    var fn = new Function(widget.querySelector("script").innerText);
    fn();
}`;
}

function getAllWidgetNames(name) {
    return fs.readdirSync(path.join(__dirname, "../..", "default-widget"), { withFileTypes: true }).filter((dirent) => dirent.isDirectory()).map((dirent) => dirent.name);
}

module.exports = { injectHTMLByNameScript, setStylesByNameScript, getAllWidgetNames };