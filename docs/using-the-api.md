# Using the API

Use the Octos API to do any of the following with your custom mod:
- Get playback info
- Media/playback controls
- Read and write to local storage
- Access file system
- Read and write user preferences
- Access system information
- And more

## Getting started

### CDN
Include the following script tag in your HTML file:
```html
<script src="https://unpkg.com/octos@latest/octos.js"></script>
```

### Direct download
Alternatively, you can directly download the [octos.js](https://raw.githubusercontent.com/underpig1/octos/master/octos.js) file and include it in your mod folder:
```html
<script src="octos.js"></script>
```

## Example

This simple mod uses the MediaController class to display the name of the current playing track and contains a button for skipping to the next track.

`index.html`:
```html
<html>
    <body>
        <p>Now playing: <span id="song-title"></span></p>
        <button onclick="nextTrack()">Next track</button>

        <script src="https://unpkg.com/octos@latest/octos.js"></script>
        <script>
            const controller = new octos.MediaController();

            controller.on("track", (e) => {
                document.getElementById("song-title").innerText = e.title;
            });

            function nextTrack() {
                controller.send("next-track");
            }
        </script>
    </body>
</html>
```

## Classes

The Octos API contains the following classes:
- [FileDialog](?t=file-dialog)
- [FileSystem](?t=file-system)
- [MediaController](?t=media-controller)
- [UserPreferences](?t=user-preferences)
- [Storage](?t=storage)
- [system](?t=system)

## Visit the [API reference](?t=file-dialog) to learn more.