# Getting started with the Octos API

The Octos API is a lightweight tool that can make your wallpapers even more powerful by interfacing with select native Windows functions from JavaScript.

Here's a quick (but not exhaustive) overview of the different classes provided in the Octos API:

| Class | Features |
| ---- | ---- |
| [MediaController](../../reference/mediacontroller) | <li>Access media properties like the title, thumbnail image, artist name, and much more</li><li>Listen to events for playback state changes and media timeline info</li><li>Toggle play/pause, enable shuffle, and seek</li> |
| [MonitorBridge](../../reference/monitorbridge) | <li>Communicate across instances of your wallpaper living on different monitors</li><li>Get monitor info for the system</li> |
| [System](../../reference/system) | <li>Temporarily hide desktop icons to allow a user to interact seamlessly with your wallpaper</li><li>Use web debugging tools like DevTools</li><li>Retrieve system information like the system theme</li> |
| [UserOptions](../../reference/useroptions) | <li>Listen to events related to user option changes on your mod</li><li>Get and set user options</li> |

> As always, if there's a feature you'd like to see in the API, please don't hesistate to submit a [feature request](https://github.com/underpig1/octos/issues). Don't be shy - the continued development of Octos depends on them! They are always appreciated.

All Octos API member classes are thoroughly documented with examples in the [API reference](../../reference).

## Using the API
The Octos API is super easy to use! Let's get started by adding it to your mod using either of the two following options. These will expose the `octos` namespace to your runtime, which includes all of the API member classes. 
#### Option 1: CDN (recommended)
Include this script tag in your HTML:
```html
<script src="https://unpkg.com/octos@latest/octos.min.js"></script>
```
#### Option 2: direct download
Alternatively, you can download [`octos.min.js`](https://unpkg.com/octos@latest/octos.min.js) from the source and include it directly in your wallpaper's source folder:
```html
<script src="js/octos.min.js"></script>
```

## Example: make a simple media player
If you need some inspiration, let's get you started with a simple fully functional example. We're going to make a media player that allows the user to pause/play media and enable/disable shuffle with the MediaController class.

#### Preview
<div style="font-family: sans-serif; text-align: center; padding: 2em; padding-top: 1px; width: 300px; background-color: darkgray; margin-bottom: 20px">
<h2 style="color: black">Media Player</h2>
<button id="playPauseBtn" style="margin-top: 1em; font-weight: bold;">Play</button>
<button id="shuffleBtn" style="margin-top: 1em; font-weight: bold;">Shuffle Off</button>
<div id="status" style="margin-top: 1em; font-weight: bold; color: black; font-weight: normal;">Status: Paused | Shuffle: On</div>
</div>

You can style your media player however you want, but hopefully you make it look nicer than I did here!

#### Code
```js
const playPauseBtn = document.getElementById('playPauseBtn');
const shuffleBtn = document.getElementById('shuffleBtn');
const statusDiv = document.getElementById('status');

const mediaController = new octos.MediaController(); // bring in MediaController class

var shuffle = false; // keep track of whether shuffle is enabled or not

// add event handlers for toggling our pause/play and shuffle buttons
playPauseBtn.addEventListener('click', () => mediaController.togglePlayPause());
shuffleBtn.addEventListener('click', () => mediaController.setShuffle(shuffle = !shuffle));

// update our text components
function updateStatus(playbackInfo) {
    statusDiv.textContent = `Status: ${info.playbackStatus} | Shuffle: ${info.shuffleActive ? "On" : "Off"}`;
    playPauseBtn.textContent = info.playbackStatus === "Playing" ? "Pause" : "Play";
    shuffleBtn.textContent = info.shuffleActive ? "Shuffle On" : "Shuffle Off";

    shuffle = info.shuffleActive; // update shuffle state
}

// add an event listener for playback state changes (ex. plause/play, shuffle changes)
mediaController.on('playbackChange', playbackInfo => updateStatus(playbackInfo));

// intitialize status: request the current playback state at the beginning
mediaController.requestPlaybackInfo().then(playbackInfo => updateStatus(playbackInfo));
```

This is kind of ugly, so hopefully you jazz it up a bit before you actually use it in a project. This is really just the beginning of what you can do with the MediaController class. Some more things you can add is a timeline to show the user how far along they are in their music, add a thumbnail image for current playing media, and so much more with just `octos.MediaController`.

## Next steps
Consult the [API reference](../../reference) for documentation and examples of all the Octos API member classes. Happy creating! Remember to [share your mod](publish) with the world when you're finished.