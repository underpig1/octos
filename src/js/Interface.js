/**
 * @class Interface
 * Interface with a host WebView2 instance.
 * @ignore
 */
class Interface {
    constructor() {
        this._listeners = {};
        this._pendingRequests = new Map();
        this._init();
    }

    _init() {
        if (window.chrome?.webview) {
            window.chrome.webview.addEventListener('message', (e) => {
                this._handleWebviewMessage(e.data);
            });
        } else {
            console.warn(`Failed to initialize, could not attach to a WebView2 instance. ${this.constructor.name} requires a WebView2 environment.`);
        }
    }

    _on(eventName, callback) {
        if (!this._listeners[eventName]) this._listeners[eventName] = [];
        this._listeners[eventName].push(callback);
    }

    _once(eventName, callback) {
        const wrapper = (data) => {
            this.off(eventName, wrapper);
            callback(data);
        };
        this.on(eventName, wrapper);
        this._subscribe(eventName);
    }

    _off(eventName, callback) {
        if (!this._listeners[eventName]) return;
        this._listeners[eventName] = this._listeners[eventName].filter(cb => cb !== callback);
    }

    _emit(eventName, data) {
        if (!this._listeners[eventName]) return;
        for (const cb of this._listeners[eventName]) {
            cb(data);
        }
    }

    _handleWebviewMessage(msg) {
        if (!msg || typeof msg.type !== 'string') return;

        if (msg.type === 'response' && msg.requestId) {
            const pending = this._pendingRequests.get(msg.requestId);
            if (pending) {
                pending.resolve(msg.data);
                this._pendingRequests.delete(msg.requestId);
            }
            return;
        }
        else if (msg.type == 'event' && typeof this._handleReceiveEvent === 'function')
            this._handleReceiveEvent(msg);
        else if (msg.type in this._listeners) {
            this._emit(msg.type, msg.data);
        }
    }

    _request(requestType, data = {}) {
        return new Promise((resolve, reject) => {
            const requestId = crypto.randomUUID?.() || Math.random().toString(36).slice(2);
            this._pendingRequests.set(requestId, { resolve, reject });

            try {
                window.chrome?.webview?.postMessage({
                    type: 'request',
                    requestType,
                    requestId,
                    data
                });
            } catch (e) {
                this._pendingRequests.delete(requestId);
                reject(e);
            }
        });
    }

    _command(commandType, data) {
        window.chrome?.webview?.postMessage({
            type: 'command',
            commandType,
            data
        });
    }

    _subscribe(eventType) {
        window.chrome?.webview?.postMessage({
            type: 'subscribe',
            eventType
        });
    }
}

export { Interface }