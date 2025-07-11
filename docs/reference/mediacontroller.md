## Classes

<dl>
<dt><a href="#MediaController">MediaController</a></dt>
<dd></dd>
</dl>

## Typedefs

<dl>
<dt><a href="#MediaProperties">MediaProperties</a> : <code>Object</code></dt>
<dd></dd>
<dt><a href="#PlaybackInfo">PlaybackInfo</a> : <code>Object</code></dt>
<dd></dd>
<dt><a href="#TimelineProperties">TimelineProperties</a> : <code>Object</code></dt>
<dd></dd>
</dl>

<a name="MediaController"></a>

## MediaController
**Kind**: global class  

* [MediaController](#MediaController)
    * [new MediaController()](#new_MediaController_new)
    * [.on(eventName, callback)](#MediaController+on)
    * [.once(eventName, callback)](#MediaController+once)
    * [.off(eventName, callback)](#MediaController+off)
    * [.requestMediaProperties()](#MediaController+requestMediaProperties) ⇒ [<code>Promise.&lt;MediaProperties&gt;</code>](#MediaProperties)
    * [.requestPlaybackInfo()](#MediaController+requestPlaybackInfo) ⇒ [<code>Promise.&lt;PlaybackInfo&gt;</code>](#PlaybackInfo)
    * [.requestTimelineProperties()](#MediaController+requestTimelineProperties) ⇒ [<code>Promise.&lt;TimelineProperties&gt;</code>](#TimelineProperties)
    * [.requestThumbnail()](#MediaController+requestThumbnail) ⇒ <code>Promise.&lt;string&gt;</code>
    * [.play()](#MediaController+play)
    * [.pause()](#MediaController+pause)
    * [.stop()](#MediaController+stop)
    * [.togglePlayPause()](#MediaController+togglePlayPause)
    * [.skipNext()](#MediaController+skipNext)
    * [.skipPrevious()](#MediaController+skipPrevious)
    * [.setShuffle(state)](#MediaController+setShuffle)
    * [.setRepeatMode(mode)](#MediaController+setRepeatMode)
    * [.setSeekPosition(position)](#MediaController+setSeekPosition)

<a name="new_MediaController_new"></a>

### new MediaController()
Handle and control events related to system media and playback.

<a name="MediaController+on"></a>

### mediaController.on(eventName, callback)
Add a listener to changes in media events.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type | Description |
| --- | --- | --- |
| eventName | <code>&#x27;mediaChange&#x27;</code> \| <code>&#x27;playbackChange&#x27;</code> \| <code>&#x27;timelineChange&#x27;</code> | - Events with type `mediaChange` are fired when the current playing media changes (ex. skipping to the next song). - Events with type `playbackChange` are fired when the media playback state changes (ex. pausing/playing a song, enabling shuffle, etc.). - Events with type `timelineChange` are fired whenever the timestamp of the current playing media changes (ex. seeking ahead or back, song progressing, etc.). |
| callback | <code>function</code> | Callback receives an object containing one of the following: - `mediaChange`: [MediaProperties](#MediaProperties) - `playbackChange`: [PlaybackInfo](#PlaybackInfo) - `timelineChange`: [TimelineProperties](#TimelineProperties) |

**Example**  
```js
mediaController.on('mediaChange', (mediaProps) => {     console.log('Currently playing: ' + mediaProps.title);});mediaController.on('playbackChange', (playbackInfo) => {     if (playbackInfo.playbackStatus == 'Paused')         console.log('Media is paused.');});
```
<a name="MediaController+once"></a>

### mediaController.once(eventName, callback)
Add a one-time event listener that removes itself after firing.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;mediaChange&#x27;</code> \| <code>&#x27;playbackChange&#x27;</code> \| <code>&#x27;timelineChange&#x27;</code> | 
| callback | <code>function</code> | 

<a name="MediaController+off"></a>

### mediaController.off(eventName, callback)
Remove an event listener.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;mediaChange&#x27;</code> \| <code>&#x27;playbackChange&#x27;</code> \| <code>&#x27;timelineChange&#x27;</code> | 
| callback | <code>function</code> | 

<a name="MediaController+requestMediaProperties"></a>

### mediaController.requestMediaProperties() ⇒ [<code>Promise.&lt;MediaProperties&gt;</code>](#MediaProperties)
Request all media properties.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
**Returns**: [<code>Promise.&lt;MediaProperties&gt;</code>](#MediaProperties) - Resolves to a MediaProperties object.  
**Example**  
```js
mediaController.requestMediaProperties().then((mediaProps) => {     console.log('Currently playing: ' + mediaProps.title);})
```
<a name="MediaController+requestPlaybackInfo"></a>

### mediaController.requestPlaybackInfo() ⇒ [<code>Promise.&lt;PlaybackInfo&gt;</code>](#PlaybackInfo)
Request all playback info.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
**Returns**: [<code>Promise.&lt;PlaybackInfo&gt;</code>](#PlaybackInfo) - Resolves to a PlaybackInfo object.  
<a name="MediaController+requestTimelineProperties"></a>

### mediaController.requestTimelineProperties() ⇒ [<code>Promise.&lt;TimelineProperties&gt;</code>](#TimelineProperties)
Request all timeline properties.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
**Returns**: [<code>Promise.&lt;TimelineProperties&gt;</code>](#TimelineProperties) - Resolves to a TimelineProperties object.  
<a name="MediaController+requestThumbnail"></a>

### mediaController.requestThumbnail() ⇒ <code>Promise.&lt;string&gt;</code>
Request the thumbnail image of the current playing media.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
**Returns**: <code>Promise.&lt;string&gt;</code> - Resolves to a base64-encoded image string.  
**Example**  
```js
myImage = document.getElementById('img');mediaController.requestThumbnail().then((thumbnail) => {     myImage.src = thumbnail;     console.log('Received thumbnail image');});
```
<a name="MediaController+play"></a>

### mediaController.play()
Play the current media.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
<a name="MediaController+pause"></a>

### mediaController.pause()
Pause the current media.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
<a name="MediaController+stop"></a>

### mediaController.stop()
Stop the current media.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
<a name="MediaController+togglePlayPause"></a>

### mediaController.togglePlayPause()
Toggle play/pause. If the current playback state is paused, this will play the current media, and vice versa.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
<a name="MediaController+skipNext"></a>

### mediaController.skipNext()
Skip to the next media.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
<a name="MediaController+skipPrevious"></a>

### mediaController.skipPrevious()
Skip to the previous media.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
<a name="MediaController+setShuffle"></a>

### mediaController.setShuffle(state)
Set the shuffle state.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| state | <code>boolean</code> | <code>true</code> | The shuffle state. Set to `true` to enable shuffling, `false` to disable it. |

<a name="MediaController+setRepeatMode"></a>

### mediaController.setRepeatMode(mode)
Set the repeat mode.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type | Description |
| --- | --- | --- |
| mode | <code>&#x27;track&#x27;</code> \| <code>&#x27;list&#x27;</code> \| <code>&#x27;none&#x27;</code> | The repeat mode. `track` enables repeat for the current track, `list` enables repeat for the current media's playlist/album, `none` disables repeat. |

<a name="MediaController+setSeekPosition"></a>

### mediaController.setSeekPosition(position)
Set the seek position.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type | Description |
| --- | --- | --- |
| position | <code>number</code> | The seek position of playback in the current media in seconds. |

<a name="MediaProperties"></a>

## MediaProperties : <code>Object</code>
**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| title | <code>string</code> | The title of the current media. |
| albumArtist | <code>string</code> | The album artist of the current media. |
| albumTitle | <code>string</code> | The album title of the current media. |
| albumTrackCount | <code>number</code> | The number of tracks in the album of the current media. |
| artist | <code>string</code> | The name of the artist of the current media. |
| genres | <code>Array.&lt;string&gt;</code> | A list of the current media's genres. |
| trackNumber | <code>number</code> | The track number of the current media. |

<a name="PlaybackInfo"></a>

## PlaybackInfo : <code>Object</code>
**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| playbackStatus | <code>&#x27;Closed&#x27;</code> \| <code>&#x27;Opened&#x27;</code> \| <code>&#x27;Changing&#x27;</code> \| <code>&#x27;Stopped&#x27;</code> \| <code>&#x27;Playing&#x27;</code> \| <code>&#x27;Paused&#x27;</code> \| <code>&#x27;Unknown&#x27;</code> | The status of the current media's playback. |
| playbackRate | <code>number</code> | The playback rate of the current media (ex. 1.5 for 1.5x speed-up playback). |
| shuffleActive | <code>boolean</code> | Whether or not the current media has shuffle enabled. |
| playbackType | <code>&#x27;Music&#x27;</code> \| <code>&#x27;Image&#x27;</code> \| <code>&#x27;Video&#x27;</code> \| <code>&#x27;Unknown&#x27;</code> | The type of the current media. |

<a name="TimelineProperties"></a>

## TimelineProperties : <code>Object</code>
**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| startTime | <code>number</code> | The starting time of the current media in seconds. |
| endTime | <code>number</code> | The ending time of the current media in seconds. |
| position | <code>number</code> | The seek position of playback in the current media in seconds (ex. a user is N seconds into a song). |
| minSeekTime | <code>number</code> | The minimum seek time of playback in the current media in seconds. |
| maxSeekTime | <code>number</code> | The maximum seek time of playback in the current media in seconds (ex. the length of the media is `maxSeekTime - minSeekTime`). |

