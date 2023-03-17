const { ipcRenderer, contextBridge } = require("electron");

// contextBridge.exposeInMainWorld("mouse", {
//     position: wp.mousePosition,
//     pressed:  wp.leftMousePressed,
//     active: wp.inForeground
// });

// contextBridge.exposeInMainWorld("system", {
//     setTaskbar: (state) => wp.setTaskbar(state)
// });

function attachRenderer(path) {
    const script = window.document.createElement("script");
    script.src = path;
    window.document.head.appendChild(script);
}

ipcRenderer.on("path", (event, content) => {
    if (window.document.readyState == "loading") window.document.addEventListener("DOMContentLoaded", () => attachRenderer(content));
    else attachRenderer(content)
});

async function mouse() {
    const result = await ipcRenderer.invoke("mouse");
    return result;
}

contextBridge.exposeInMainWorld("mouse", mouse);