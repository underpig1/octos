window.chrome.webview.addEventListener('message', event => {
    switch (event.data.type) {
        case "ping":
            console.log("Got ping:", event.data.payload);
            break;
        default:
            break;
    }
});

