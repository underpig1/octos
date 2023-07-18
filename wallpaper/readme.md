# Octos/Wallpaper

Wallpaper is the native node module that powers Octos.

```
./node_modules/.bin/electron-rebuild.cmd
cd wallpaper&npm install
```
```javascript
const wp = require("wallpaper")
```

### Quick API Reference

| Function | Description
| ----- | ----- |
| `attach(win)` | Attaches given window to the desktop | 
| `detach(win)` | Detaches given window from the desktop |
| `mousePosition()` | Returns mouse postion as array |
| `leftMousePressed()` | Returns true if left mouse button pressed |
| `leftMousePressed()` | Returns mouse postion as array |
| `inForeground()` | Returns true if the attached window is in the foreground |
| `setTaskbar(state = true)` | Enables/disables taskbar visibility |
| `inForeground()` | Returns true if the attached window is focused |
| `middleMousePressed()` | Returns true if the middle mouse button is pressed |
| `rightMousePressed()` | Returns true if the right mouse button is pressed |
| `keyboard()` | Returns array of keyboard states |
| `sendMediaEvent(state = 0)` | Sends media event (-1 -> prev track; 0 -> play/pause; 1 -> next track; else, calls keyboard event for state key -> refer to [Windows Virtual Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)) |
| `fgTitle()` | Returns the title of the foreground window |
| `trackTitle()` | Returns the title of the playing track |
| `trackArtist()` | Returns the artist of the playing track |
| `trackTimeline()` | Returns the timeline of the playing track |
| `playbackStatus()` | Returns the playback status of the playing track (playing, paused, stopped, changing, closed, opened) |

# Build errors? Remove `node_modules/`, `bin/`, and `build/` before running `npm install` again