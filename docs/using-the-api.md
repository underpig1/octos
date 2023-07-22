# Using the API

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
                controller.nextTrack();
            }
        </script>
    </body>
</html>
```

## Contents

The Octos API contains the following features:
- [FileDialog](?t=file-dialog) - Request files/directories from the user
- [FileSystem](?t=file-system) - Access user's file system
- [MediaController](?t=media-controller) - Send requests and read media playback data
- [UserPreferences](?t=user-preferences) - Read and write preferences for your mod
- [Storage](?t=storage) - Read and write to local storage
- [system](?t=system) - Access system-level information

Visit the [API reference](?t=file-dialog) to learn more.