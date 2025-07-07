## Classes

<dl>
<dt><a href="#MediaController">MediaController</a></dt>
<dd><p>Handle and control events related to system media and playback.</p>
</dd>
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
Handle and control events related to system media and playback.

**Kind**: global class  

* [MediaController](#MediaController)
    * [.on(eventName, callback)](#MediaController+on)
    * [.once(eventName, callback)](#MediaController+once)
    * [.off(eventName, callback)](#MediaController+off)
    * [.request()](#MediaController+request) ⇒ <code>Promise.&lt;any&gt;</code>

<a name="MediaController+on"></a>

### mediaController.on(eventName, callback)
Add a listener to changes in media events.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type | Description |
| --- | --- | --- |
| eventName | <code>&#x27;change&#x27;</code> \| <code>&#x27;playback&#x27;</code> \| <code>&#x27;timeline&#x27;</code> | <ul> <li>Events with type `change` are fired when the current playing media changes (ex. skipping to the next song).</li> <li>Events with type `playback` are fired when the media playback state changes (ex. pausing/playing a song, enabling shuffle, etc.).</li> <li>Events with type `timeline` are fired whenever the timestamp of the current playing media changes (ex. seeking ahead or back, song progressing, etc.).</li> </ul> |
| callback | <code>function</code> | Callback recieves an object containing one of the following: <ul> <li>`change`: [MediaProperties](#mediaproperties)</li> <li>`playback`: [PlaybackInfo](#playbackinfo)</li> <li>`timeline`: [TimelineProperties](#timelineproperties)</li> </ul> |

**Example**  
```js
mediaController.on('change', (mediaProps) => {     console.log('Currently playing: ' + mediaProps.title);});mediaController.on('playback', (playbackInfo) => {     if (playbackInfo.playbackStatus == 'Paused')         console.log('Media is paused.');});
```
<a name="MediaController+once"></a>

### mediaController.once(eventName, callback)
Add a one-time event listener that removes itself after firing.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;change&#x27;</code> \| <code>&#x27;load&#x27;</code> | 
| callback | <code>function</code> | 

<a name="MediaController+off"></a>

### mediaController.off(eventName, callback)
Remove an event listener.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;change&#x27;</code> \| <code>&#x27;load&#x27;</code> | 
| callback | <code>function</code> | 

<a name="MediaController+request"></a>

### mediaController.request() ⇒ <code>Promise.&lt;any&gt;</code>
Request all user options along with their properties and values and await a response.

**Kind**: instance method of [<code>MediaController</code>](#MediaController)  
**Returns**: <code>Promise.&lt;any&gt;</code> - Resolves to an object containing `options` from `octos.json` along with user-set values.  
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
| shuffleActie | <code>bool</code> | Whether or not the current media has shuffle enabled. |
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
| maxSeekTime | <code>number</code> | The maximum seek time of playback in the current media in seoconds (ex. the length of the media is `maxSeekTime - minSeekTime`). |

