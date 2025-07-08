import { Interface } from './Interface.js'

/**
 * Send and recieve messages between instances of your wallpaper across monitors if your wallpaper is running in a multimonitor environment.
 * Each instance of your wallpaper is assigned to a unique monitor and thus has a unique monitor ID string (something like `"\\\\.\\DISPLAY1"`). You can use these IDs to send messages between separate instances of your wallpaper.
 * This is useful for coordinating events between different instances if you want your mod to behave differently across monitors.
 */
class MonitorBridge extends Interface {
    constructor() {
        super();
        this._listeners = {
            message: []
        };
    }

    /**
     * @typedef {Object} MonitorMessage
     * @property {string} senderId - The monitor ID of the sender.
     * @property {Object} data - The contents of the message being recieved.
     */

    _handleReceiveEvent(msg) {
        if (!msg.data)
            return;
        switch (msg.eventType) {
            case 'redirect':
                this._emit('message', {
                    senderId: msg.data['sender-id'],
                    data: msg.data.message
                });
                break;
            default:
                break;
        }
    }

    /**
     * Request this window's monitor ID.
     * @returns {Promise<string>} Resolves to a string containing this window's monitor ID.
     * @example
     * monitorBridge.requestId().then((myId) => {
     *      console.log('My monitor id is: ' + myId); // probably will look like "\\\\.\\DISPLAY1"
     * });
     */
    async requestId() {
        return super._request('monitor-id');
    }

    /**
     * Request the monitor IDs of sibling windows. A sibling is another window running on another monitor but running the same mod instance as yours. That is, it will include wallpapers in other monitors only if they are running your wallpaper. Monitors assigned to wallpapers are excluded. 
     * @returns {Promise<string[]>} Resolves to an array of strings containing the monitor ID strings of this window's siblings.
     * @example
     * monitorBridge.requestSiblingIds().then((siblingIds) => {
     *      for (const siblingId of siblingIds) {
     *          console.log('Sibling ID: ' + siblingId)
     *          // do something with each one
     *      }
     *      console.log('We have ' + siblingIds.length + ' simultaneous instances of our wallpaper running on different monitors');
     * }
     */
    async requestSiblingIds() {
        return super._request('siblings');
    }

    /**
     * Send a message to a window belonging to another monitor.
     * @param {string} recipientId - The monitor ID of the intended recipient.
     * @param {Object} data - The contents of the message to send.
     * @example
     * // Send a message to all siblings
     * someData = {
     *      hello: true,
     *      myFavoriteNumber: 4
     * };
     * 
     * monitorBridge.requestSiblingIds().then((siblingIds) => {
     *      siblingIds.forEach((id) => {
     *          monitorBridge.send(id, someData);
     *      });
     * })
     */
    send(recipientId, data) {
        super._command('redirect', {
            'receiver-id': recipientId,
            message: data
        });
    }

    /**
     * Add an event listener.
     * @param {'message'} eventName - `'message'` events fire when another instance sends a message to this monitor ID.
     * @param {function(object)} callback - Resolves to a [MonitorMessage](#monitormessage) object, containing both `senderId` and `message`.
     * @example
     * monitorBridge.on('message', (e) => {
     *      console.log('Incoming message from: ' + e.senderId);
     *      console.log('Message contents: ' + JSON.stringify(e.message));
     *      // You can also respond:
     *      const recipientId = e.senderId;
     *      monitorBridge.send(recipientId, {
     *          someString: 'I got your message'
     *          someData: 3
     *      });
     * })
     */
    on(eventName, callback) {
        super._on(eventName, callback);
    }

    /**
     * Add a one-time event listener that removes itself after firing.
     * @param {'message'} eventName
     * @param {function(object)} callback
     */
    once(eventName, callback) {
        super._once(eventName, callback);
    }

    /**
     * Remove an event listener.
     * @param {'message'} eventName
     * @param {Function} callback
     */
    off(eventName, callback) {
        this._off(eventName, callback);
    }
}

export { MonitorBridge };