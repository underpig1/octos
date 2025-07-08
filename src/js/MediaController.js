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
            mediaChange: [],
            playbackChange: [],
            timelineChange: []
        };
    }

    _handleReceiveEvent(msg) {
        switch (msg.eventType) {
            case 'media-change':
                this._emit('mediaChange', msg.data);
                break;
            case 'playback-change':
                this._emit('playbackchange', msg.data);
                break;
            case 'timeline-change':
                this._emit('timelinechange', msg.data);
                break;
            default:
                break;
        }
    }

    /**
     * Add a listener to changes in media events.
     * @param { 'mediaChange' | 'playbackChange' | 'timelineChange' } eventName
     * <ul>
     * <li>Events with type `mediaChange` are fired when the current playing media changes (ex. skipping to the next song).</li>
     * <li>Events with type `playbackchange` are fired when the media playback state changes (ex. pausing/playing a song, enabling shuffle, etc.).</li>
     * <li>Events with type `timelinechange` are fired whenever the timestamp of the current playing media changes (ex. seeking ahead or back, song progressing, etc.).</li>
     * </ul>
     * @param {function(object)} callback
     * Callback recieves an object containing one of the following:
     * <ul>
     * <li>`mediaChange`: [MediaProperties](#mediaproperties)</li>
     * <li>`playbackchange`: [PlaybackInfo](#playbackinfo)</li>
     * <li>`timelinechange`: [TimelineProperties](#timelineproperties)</li>
     * </ul>
     * @example
     * mediaController.on('mediaChange', (mediaProps) => {
     *      console.log('Currently playing: ' + mediaProps.title);
     * });
     * 
     * mediaController.on('playbackchange', (playbackInfo) => {
     *      if (playbackInfo.playbackStatus == 'Paused')
     *          console.log('Media is paused.');
     * });
     */
    on(eventName, callback) {
        super._on(eventName, callback);
    }

    /**
     * Add a one-time event listener that removes itself after firing.
     * @param {'mediaChange' | 'playbackChange' | 'timelineChange'} eventName
     * @param {function(object)} callback
     */
    once(eventName, callback) {
        super._once(eventName, callback);
    }

    /**
     * Remove an event listener.
     * @param {'mediaChange' | 'playbackChange' | 'timelineChange'} eventName
     * @param {Function} callback
     */
    off(eventName, callback) {
        this._off(eventName, callback);
    }

    /**
     * Request all media properties.
     * @returns {Promise<object>} Resolves to a [MediaProperties](#mediaproperties) object.
     * @example
     * mediaController.getMediaProperties().then((mediaProps) => {
     *      console.log('Currently playing: ' + mediaProps.title);
     * })
     */
    async requestMediaProperties() {
        return super._request('media-props');
    }

    /**
     * Request all playback info.
     * @returns {Promise<object>} Resolves to a [PlaybackInfo](#playbackinfo) object.
     */
    async requestPlaybackInfo() {
        return super._request('playback-info');
    }

    /**
     * Request all timeline properties.
     * @returns {Promise<object>} Resolves to a [TimelineProperties](#timelineproperties) object.
     */
    async requestTimelineProperties() {
        return super._request('timeline-props');
    }

    /**
     * Request the thumbnail image of the current playing media.
     * @returns {Promise<string>} Resolves to a base64-encoded image string.
     * @example
     * myImage = document.getElementById('img');
     * mediaController.getThumbnail().then((thumbnail) => {
     *      myImage.src = thumbnail;
     *      console.log('Recieved thumbnail image');
     * });
     */
    async requestThumbnail() {
        return super._request('thumbnail');
    }

    /**
     * Play the current media.
     */
    play() {
        return super._command('media', {cmd: 'play'});
    }

    /**
     * Pause the current media.
     */
    pause() {
        return super._command('media', { cmd: 'pause' });
    }

    /**
     * Stop the current media.
     */
    stop() {
        return super._command('media', { cmd: 'stop' });
    }

    /**
     * Toggle play/pause. If the current playback state is paused, this will play the current media, and vice versa. 
     */
    togglePlayPause() {
        return super._command('media', { cmd: 'toggle' });
    }

    /**
     * Skip to the next media.
     */
    skipNext() {
        return super._command('media', { cmd: 'next' });
    }

    /**
     * Skip to the previous media.
     */
    skipPrevious() {
        return super._command('media', { cmd: 'previous' });
    }

    /**
     * Set the shuffle state.
     * @param {bool} state - The suffle state. Set to `true` to enable shuffling, `false` to disable it.
     */
    setShuffle(state = true) {
        if (state) super._command('media', { cmd: 'enable-shuffle' });
        else super._command('media', { cmd: 'disable-shuffle' });
    }

    /**
     * Set the repeat mode.
     * @param {'track' | 'list' | 'none'} mode - The repeat mode. `track` enables repeat for the current track, `list` enables repeat for the current media's playlist/album, `none` disables repeat.
     */
    setRepeatMode(mode) {
        switch (mode) {
            case 'track':
                super._command('media', { cmd: 'set-repeat-track' });
                break;
            case 'list':
                super._command('media', { cmd: 'set-repeat-list' });
                break;
            case 'none':
                super._command('media', { cmd: 'set-repeat-none' });
                break;
            default:
                break;
        }
    }

    /**
     * Set the seek position.
     * @param {int} position - The seek position of playback in the current media in seconds.
     */
    setSeekPosition(position) {
        if (Number.isInteger(position)) super._command('media', { cmd: 'set-playback-position' });
        else throw new TypeError('Expected an integer');
    }
}

export { MediaController };