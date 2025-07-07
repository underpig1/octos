import { Interface } from './Interface.js'

/**
 * Configure the WebView2 session for debugging and other options.
 */
class WebView extends Interface {
    constructor() {
        super();
    }

    /**
     * Open the DevTools window to debug your mod.
     * @example
     * const webview = new octos.WebView();
     * webview.enableDevTools();
     */
    enableDevTools() {
        super.command('open-dev-tools');
    }

    /**
     * Get the source uri of the current document.
     * @returns { Promise<string> } Resolves to the document's uri.
     * @example
     * webview.getSource().then((uri) => alert(uri))
    */
    async getSource() {
        return super.request('source');
    }
}

export { WebView };