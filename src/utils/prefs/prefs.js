function createPrefsWindow(prefs) {
    for (var id of Object.keys(prefs)) {
        
    }
}

function createElement(content) {
    var type = content.type;
    if (type) {
        var label = content.label;
        var value = content.value | 0;
        var min = content.min;
        var max = content.max;
        var step = content.step;
        switch (type) {
            case "color":
                break;
            case "file":
                break;
            case "slider":
                break;
            case "text":
                break;
            case "number":
                break;
            case "checkbox":
                break;
            case "label":
                break;
            default:
                break;
        }
    }
}