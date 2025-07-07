import { Interface } from './Interface.js'

/**
 * Handle and control events related to system media and playback.
 */
class MediaController extends Interface {
    /**
     * @typedef {Object} MediaProperties
     * @property {string} title - The title of the current media.
     * @property {string} albumArtist - The album artist of the current media.
     * @property {string} albumTitle - The album title of the current media.
     * @property {number} albumTrackCount - The number of tracks in the album of the current media.
     * @property {string} artist - The name of the artist of the current media.
     * @property {Array<string>} genres - A list of the current media's genres.
     * @property {number} trackNumber - The track number of the current media.
     */

    /**
     * @typedef {Object} PlaybackInfo
     * @property {'Closed' | 'Opened' | 'Changing' | 'Stopped' | 'Playing' | 'Paused' | 'Unknown'} playbackStatus - The status of the current media's playback.
     * @property {number} playbackRate - The playback rate of the current media (ex. 1.5 for 1.5x speed-up playback).
     * @property {bool} shuffleActie - Whether or not the current media has shuffle enabled.
     * @property {'Music' | 'Image' | 'Video' | 'Unknown'} playbackType - The type of the current media.
     */

    /**
     * @typedef {Object} TimelineProperties
     * @property {number} startTime - The starting time of the current media in seconds.
     * @property {number} endTime - The ending time of the current media in seconds.
     * @property {number} position - The seek position of playback in the current media in seconds (ex. a user is N seconds into a song).
     * @property {number} minSeekTime - The minimum seek time of playback in the current media in seconds.
     * @property {number} maxSeekTime - The maximum seek time of playback in the current media in seoconds (ex. the length of the media is `maxSeekTime - minSeekTime`).
     */

    constructor() {
        super();
        this._listeners = {
            change: [],
            playback: [],
            timeline: []
        };
    }

    _handleReceiveEvent(msg) {
        switch (msg.eventType) {
            case 'media-change':
                this._emit('changes', msg.data);
                break;
            case 'playback-change':
                this._emit('playback', msg.data);
                break;
            case 'timeline-change':
                this._emit('timeline', msg.data);
                break;
            default:
                break;
        }
    }

    /**
     * Add a listener to changes in media events.
     * @param { 'change' | 'playback' | 'timeline' } eventName
     * <ul>
     * <li>Events with type `change` are fired when the current playing media changes (ex. skipping to the next song).</li>
     * <li>Events with type `playback` are fired when the media playback state changes (ex. pausing/playing a song, enabling shuffle, etc.).</li>
     * <li>Events with type `timeline` are fired whenever the timestamp of the current playing media changes (ex. seeking ahead or back, song progressing, etc.).</li>
     * </ul>
     * @param {function(object)} callback
     * Callback recieves an object containing one of the following:
     * <ul>
     * <li>`change`: [MediaProperties](#mediaproperties)</li>
     * <li>`playback`: [PlaybackInfo](#playbackinfo)</li>
     * <li>`timeline`: [TimelineProperties](#timelineproperties)</li>
     * </ul>
     * @example
     * mediaController.on('change', (mediaProps) => {
     *      console.log('Currently playing: ' + mediaProps.title);
     * });
     * 
     * mediaController.on('playback', (playbackInfo) => {
     *      if (playbackInfo.playbackStatus == 'Paused')
     *          console.log('Media is paused.');
     * });
     */
    on(eventName, callback) {
        super._on(eventName, callback);
    }

    /**
     * Add a one-time event listener that removes itself after firing.
     * @param {'change' | 'load'} eventName
     * @param {function(object)} callback
     */
    once(eventName, callback) {
        super._once(eventName, callback);
    }

    /**
     * Remove an event listener.
     * @param {'change' | 'load'} eventName
     * @param {Function} callback
     */
    off(eventName, callback) {
        this._off(eventName, callback);
    }

    /**
     * Request all user options along with their properties and values and await a response.
     * @returns {Promise<any>} Resolves to an object containing `options` from `octos.json` along with user-set values.
     */
    async request() {
        return super._request('options');
    }
}

export { Media };