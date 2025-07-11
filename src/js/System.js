import { Interface } from './Interface.js'

/**
 * @class System
 * @description
 * Configure system options and request system information.
 */
class System extends Interface {
    constructor() {
        super();
    }

    /**
     * Open the DevTools window for mod debugging.
     * @example
     * const system = new octos.System();
     * system.enableDevTools();
     */
    enableDevTools() {
        super._command('open-dev-tools');
    }

    /**
     * Get the source uri of the current document.
     * @returns { Promise<string> } Resolves to the document's uri.
     * @example
     * system.getSource().then((uri) => alert(uri))
    */
    async getSource() {
        return super._request('source');
    }

    /**
     * Get the system theme (light or dark).
     * @returns { Promise<string> } Resolves to either `'light'` or `'dark'`.
     * @example
     * system.getTheme().then((theme) => alert(theme))
    */
    async getTheme() {
        const light = await super._request('is-theme-light');
        return light ? 'light' : 'dark';
    }

    /**
     * Set the wallpaper's visibility.
     * @param {bool} state - `true` for visible, `false` for hidden.
    */
    setVisibility(state) {
        if (state === true)
            super._command('set-visible');
        else if (state === false)
            super._command('set-hidden');
        else
            throw TypeError('Expected boolean');
    }

    /**
     * Request the wallpaper's visibility.
     * @returns {Promise<bool>} Resolves to `true` for visible, `false` for hidden.
    */
    async getVisibility() {
        return super._request('visibility');
    }
    
    /**
     * Set the visibility of desktop icons. Useful for hiding the desktop icons when mouse input is expected and then re-enabling them when finished.
     * @param {bool} state - `true` for visible, `false` for hidden.
    */
    setDesktopIconVisibility(state) {
        if (state === true)
            super._command('set-desktop-icons-visible');
        else if (state === false)
            super._command('set-desktop-icons-hidden');
        else
            throw TypeError('Expected boolean');
    }

    /**
     * Request the visibility of desktop icons.
     * @returns {Promise<bool>} Resolves to `true` for visible, `false` for hidden.
    */
    async getDesktopIconVisibility() {
        return super._request('desktop-icons-visibility');
    }
}

export { System };