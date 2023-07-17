# Widgets (more coming soon!)

Use a library of premade widgets directly in your mods to build your own wallpaper fast.

It's easy to use:

Include the `widget` element in your HTML and set the `name` attribute to one of the existing widgets in the library. Widgets can be styled just like normal elements, but some widgets include special CSS properties which need to be specified as variables.

CSS:
```css
.media {
    width: 200px;
    height: 100px;
    --offset: 50px;
}
```
HTML:
```html
<widget class="media" name="media-player"></widget>
```

## Media player

Renders a simple media player with play, pause, and track controls as well as a time elapsed slide tracker.

```html
<widget name="media-player"></widget>
```

Special CSS properties:
- `--color` color - The accent color of the media player (not all elements are styled in this way)
- `--offset` length - The distance between the track control buttons and the edge of the widget