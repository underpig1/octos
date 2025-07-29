# Tips and Tricks
Check out these tips to help you make your mods even more legendary.

## Save state
If you're making a complex mod like a game, you may want to save the state in between sessions so the user can jump right back in when they come back.

Recall that pretty much all native JavaScript APIs are available in Octos mods, including the [localStorage](https://developer.mozilla.org/en-US/docs/Web/API/Window/localStorage) API.

As a refresher, here's a simple example for storing and loading some JSON data:
```js
// save
localStorage.setItem('myData', JSON.stringify(myData))
// load
const saved = localStorage.getItem('myData')
if (saved) {
    myData = JSON.parse(saved);
}
```

As an alternative, you could also use the [IndexedDB API](https://developer.mozilla.org/en-US/docs/Web/API/IndexedDB_API) for a more complex structured local database or [cookies](https://developer.mozilla.org/en-US/docs/Web/API/Document/cookie) for very small and simple unstructured data.

## Canvas Performance
If you are using continuous `requestAnimationFrame` calls for animating the canvas, you might notice high CPU usage on some lower-end devices. This isn't always an issue since Octos pauses wallpapers when they're hidden behind a fullscreen app, so mods are generally active only when the user wants to interact with them. However, one of the goals of Octos is to make mods performant on all devices at all times, so we can do better!

One solution is animation throttling. The idea is that you want to replace the following pattern:
```js
function draw() {
    // draw everything
    requestAnimationFrame(draw);
}
draw();
```
or
```js
function draw() {
    // draw everything
}
setInterval(draw, 16); // about 60 times per second
```
with something like this:
```js
let lastTime = 0;
const targetFPS = 30; // can adjust to fit your needed FPS
const minInterval = 1000 / targetFPS;

function draw(now) {
  if (now - lastTime >= minInterval) {
    lastTime = now;
    // draw everything
  }
  requestAnimationFrame(draw);
}
draw();
```
If you have a simple animation, this is ideal for letting you reduce frame rate to something more reasonable as necessary. This can significantly reduce CPU usage on lower-end devices. Of course, this isn't always desirable if you really need something closer to 60FPS, but this is helpful in a lot of cases.

## Examples
If you want to see what cool things over people have done, check out some examples over at [Octos Community](https://github.com/underpig1/octos-community/tree/master/src).

Some specific Octos-made examples in the Community repo I recommend checking out:

- Contraption: an example of a complete and complex game-type mod that makes use of save state

- Audio: an audio player and visualizer that makes heavy use of the `MediaController` class of the Octos API

    - Did you know that you can make a frequency spectrum for Spotify, YouTube, or any other system media using `audioStream`?

- Terrain: a great example of making heavy canvas operations more performant using caching and animation throttling