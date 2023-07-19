# MediaController

Access and control the user's media playback.

```js
new MediaController()
```

## Events

### `track`

Returns: `event` containing `title` and `artist` of the playing track

Emitted when the playback track changes.

### `playbackstatus`

Returns: `event` containing `status` as a string (can be one of `"playing"`, `"paused"`, `"stopped"`)

Emitted when the playback status changes.

### `playbacktime`

Returns: `event` containing `secondsElapsed` and `secondsTotal` as integers

Emitted when the playback time changes.

## Methods
```js
controller.pausePlay()
```

Toggles the play state of media playback.
<br><br>
```js
controller.prevTrack()
```

Changes playback to the previous track in the queue.
<br><br>
```js
controller.nextTrack()
```

Changes playback to the next track in the queue.
<br><br>
```js
controller.getTrack()
```

Returns: `Promise<Object>` - Resolves as an Object containing the following track data:
- `title` string - The title of the track
- `artist` string - The artist of the track
- `secondsElapsed` number - The time in seconds elapsed
- `secondsTotal` number - The total time in seconds
- `playing` boolean - The play state of the track

Requests data for the current track.
<br><br>
```js
controller.getTrackTitle()
```

Returns: `Promise<string>` - Resolves as a string containing the title of the current track

Requests the title of the current track.
<br><br>
```js
controller.getTrackArtist()
```

Returns: `Promise<string>` - Resolves as a string containing the artist of the current track

Requests the artist of the current track.
<br><br>
```js
controller.getTrackSecondsElapsed()
```

Returns: `Promise<number>` - Resolves as a string containing the time in seconds elapsed

Requests the time in seconds elapsed on the current track.
<br><br>
```js
controller.getTrackSecondsTotal()
```

Returns: `Promise<number>` - Resolves as a number containing the total time in seconds

Requests the total time in seconds of the current track.
<br><br>
```js
controller.getTrackPercentElapsed()
```

Returns: `Promise<number>` - Resolves as a float less than or equal to 1 containing the fraction of time elapsed out of the total time on the track

Requests the percent of time that has elapsed on the current track.
<br><br>
```js
controller.isPlaying()
```

Returns: `Promise<boolean>` - Resolves as `true` if the current media is playing and `false` if it has stopped or has been paused

Requests the play state of media playback.
<br><br>
```js
controller.send([type])
```

- `type` string - The type of request to send
    - `"prev-track"` - Goes to the previous track or to the beginning of the current track if not at the beginning
    - `"pause-play` - Toggles the playback of the current media
    - `"next-track"` - Skips to the next track

Sends a request for the controller to execute a command.
<br><br>
```js
controller.request([type])
```

- `type` string - The type of information requested
    - `"title"` - Requests the title of the current track
    - `"artist` - Requests the artist of the current track
    - `"seconds-elapsed"` - Requests the time in seconds elapsed since the track began
    - `"seconds-total"` - Requests the total time in seconds in the current track
    - `"amount-elapsed"` - Requests the amount of time as a fraction of seconds elapsed out of seconds total that have passed since the track began
    - `"state"` - Requests the current playback state, returns one of `"playing"`, `"paused"`, `"stopped"`

Returns: `Promise<string>` - Resolves as the controller's response depending on the type of request

Sends a request for the controller to send back media playback info.

## Examples

Creates a simple media playback control interface with a "skip to next track" button and displays the current playing track title:

```js
const controller = new MediaController();

const trackTitle = document.createElement("h1");
document.body.appendChild(trackTitle);
controller.on("track", (e) => trackTitle.innerText = e.title);

const btn = document.createElement("button");
btn.innerText = "Skip to next track";
btn.onclick = () => controller.send("next-track");
document.body.appendChild(btn);
```