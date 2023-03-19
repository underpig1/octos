function modifier(key) {
    switch (key) {
        case 8:
            return "Backspace";
        case 9:
            return "Tab";
        case 13:
            return "Return";
        case 27:
            return "Escape";
        case 16:
            return "Shift";
        // case 160:
        //     return "ShiftUp";
        case 17:
            return "Control";
        // case 162:
        //     return "ControlUp";
        case 20:
            return "CapsLock";
        case 91:
            return "WindowsKey";
        case 18:
            return "Alt";
        case 37:
            return "Left";
        case 38:
            return "Up";
        case 39:
            return "Right";
        case 40:
            return "Down";
        case 190:
            return ".";
        default:
            return false;
    }
}

function keyCode(key, shift) {
    if (key >= 65 && key <= 90) {
        var k = String.fromCharCode(key);
        return shift ? k : k.toLowerCase();
    }
    switch (key) {
        case 49:
            return shift ? "!" : "1";
        case 50:
            return shift ? "@" : "2";
        case 51:
            return shift ? "#" : "3";
        case 52:
            return shift ? "$" : "4";
        case 53:
            return shift ? "%" : "5";
        case 54:
            return shift ? "^" : "6";
        case 55:
            return shift ? "&" : "7";
        case 56:
            return shift ? "*" : "8";
        case 57:
            return shift ? "(" : "9";
        case 48:
            return shift ? ")" : "0";
        case 189:
            return shift ? "_" : "-";
        case 187:
            return shift ? "+" : "=";
        case 219:
            return shift ? "{" : "[";
        case 221:
            return shift ? "}" : "]";
        case 220:
            return shift ? "|" : "\\";
        case 186:
            return shift ? ":" : ";";
        case 222:
            return shift ? "\"" : "'";
        case 188:
            return shift ? "<" : ",";
        case 190:
            return shift ? ">" : ".";
        case 191:
            return shift ? "?" : "/";
        case 32:
            return " ";
        default:
            return "";
    }
}

module.exports = { modifier, keyCode };