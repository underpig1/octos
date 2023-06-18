<div align="center">
<img src="img/icon.png" />
<h1>Octos - HTML Live Wallpaper Engine</h1>
<p>Create, distribute, and explore live, interactive wallpapers on Windows made with HTML, CSS, and JS.</p>
</div>

## Native API
> Interact with native Windows processes like Media Player, Local Storage, and File System
```javascript
console.log("Skipping to next track...");
window.media.nextTrack();
window.media.getTitle().then((title) => console.log(title + " is currently playing!"));

document.addEventListener("playbackstatus", (e) => {
    if (e.detail.status == "playing") console.log("Song playing!");
});
```
Learn more with the [Native API Documentation]()

## Mouse and Keyboard Support
> Full mouse and keyboard support for interactive wallpapers

## Widget Library
> Get started with a handful of premade desktop widgets
```html
<body>
    <widget name="media-player"></widget>
</body>
```

# Installation
Installation instructions

Check out the [Getting Started Guide]() to jump right in.

# Gallery