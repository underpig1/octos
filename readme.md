<div align="center">
<img src="img/tray.png" />
<h1>Octos - HTML Live Wallpaper Engine</h1>
<p>Create, distribute, and explore live, interactive wallpapers on Windows made with HTML, CSS, and JS.</p>
<img src="https://github.com/underpig1/octos/actions/workflows/ci.yml/badge.svg">
<img src="https://github.com/underpig1/octos/actions/workflows/publish.yml/badge.svg">
<img src="https://badge.fury.io/js/octos.svg">
</div>

# [Documentation](https://underpig1.github.io/octos/docs/?t=installation) | [Download](https://github.com/underpig1/octos/releases) | [Quickstart](https://underpig1.github.io/octos/docs/?t=quickstart)

<img src="img/gallery/demo.gif" alt="Demo" width="800px">

# Installation

Download the Octos app for Windows.

# Gallery

<img src="img/gallery/ethereal.gif" alt="Ethereal" width="600px">

### Ethereal
An interactive media player that ripples as you pass your mouse over it.

<img src="img/gallery/terminal.gif" alt="Terminal" width="600px">

### Terminal
A digital clock with a live old TV effect and customizable 3D text art.

<img src="img/gallery/imgbg.gif" alt="Image Background" width="600px">

### Image Background
Set your background to any image/gif/video and add widgets like a media controller, clock, and calender.

<img src="img/gallery/gradient.png" alt="Gradient" width="600px">

### Gradient
A simple analog desktop clock with a calming color-changing gradient background.

# Native API
Making your own live wallpaper is super easy with Octos' native API.

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

Use the Octos API to:
- Get playback info
- Media/playback controls
- Read and write to local storage
- Access file system
- Read and write user preferences
- Access system information
- [Learn more with the API Docs]()

# Share your Wallpaper
Once you make your own awesome wallpaper, share it with the world.