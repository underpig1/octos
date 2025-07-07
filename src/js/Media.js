import { Interface } from './Interface.js'

/**
 * Handle events relating to changes in user options for your mod.
 * 
 * User options are defined in the [`options` object in your octos.json file](config.md#options), and are configurable by users in the Octos app:
 * 
 * <img height="300px" src="_media/user-options.png" aria-hidden />
 */
class Media extends Interface {
    constructor() {
        super();
        this._listeners = {
            change: []
        };
    }

    _handleReceiveEvent(msg) {
        switch (msg.eventType) {
            case 'session-change':
                this._emit('media-change', msg.data);
                break;
            case 'media-change':
                this._emit('media-change', msg.data);
                break;
            case 'playback-change':
                this._emit('playback-change', msg.data);
                break;
            case 'timeline-change':
                this._emit('timeline-change', msg.data);
                break;
            default:
                break;
        }
    }

    /**
     * Add an event listener.
     * @param {'change' | 'load'} eventName
     * Events with type `change` are fired whenever a user changes an option. `load` events are fired when the wallpaper first loads in.
     * @param {function(object)} callback
     * Callback receives an object containing the ID of the affected option (as specified in `octos.json`) along with its value in the form `{id, value}`.
     * @example
     * // This example shows how to add a simple checkbox for dark mode in your mod and listen for changes by the user.
     * // In your octos.json:
     * {
     * ...
     *  "options": {
     *    "dark-mode": {
     *      "type": "checkbox",
     *      "value": true,
     *      "label": "Enable dark mode?"
     *    }
     *  }
     * ...
     * }
     * 
     * // In your script.js:
     * // Listen for changes in user options for your mod
     * userOptions.on('change', ({id, value}) => {
     *   if (id == 'dark-mode') {
     *      if (value)
     *          myElement.classList.add('dark-mode')
     *       }
     *       else {
     *           myElement.classList.remove('dark-mode')
     *       }
     *   }
     *   ... // handle changes for other options
     * })
     */
    on(eventName, callback) {
        super.on(eventName, callback);
    }

    /**
     * Add a one-time event listener that removes itself after firing.
     * @param {'change' | 'load'} eventName
     * @param {function(object)} callback
     */
    once(eventName, callback) {
        super.once(eventName, callback);
    }

    /**
     * Remove an event listener.
     * @param {'change' | 'load'} eventName
     * @param {Function} callback
     */
    off(eventName, callback) {
        this.off(eventName, callback);
    }

    /**
     * Request all user options along with their properties and values and await a response.
     * @returns {Promise<any>} Resolves to an object containing `options` from `octos.json` along with user-set values.
     */
    async request() {
        return super.request('options');
    }
}

export { UserOptions };