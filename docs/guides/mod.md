# Creating a mod

Creating a mod is super easy in Octos. Let's work through a simple example.

### Getting started
Start by making a folder that looks like this:
```
MyAwesomeMod/
├── index.html
├── octos.json
```
The `octos.json` file is optional, but it allows you to specify details for when you want to share your mod with the world, like name, author, HTML entry path, user options, preview image, and more.

For now, let's put this in our `octos.json`:
```json
{
    "name": "My Awesome Mod",
    "description": "This is an awesome mod.",
    "entry": "index.html"
}
```
The `entry` field basically tells Octos to load index.html as the entry point when your mod is first enabled. This field is not necessary for a simple single-page mod like this one, but it's useful if your mod later needs to navigate between different `.html` files in the same folder. `entry` can also be a link to an external webpage!

In this folder, you can include assets, libraries, icons, scripts, and anything that your mod needs to run. Let's add an `index.js` to our mod:
```
MyAwesomeMod/
├── index.html
├── index.js
├── octos.json
```

Alternatively, you can generate a starting mod folder, which includes template `index.html` and `octos.json` files with the [Octos command line interface (CLI)](cli.md):
```batch
mkdir MyAwesomeMod
octos new MyAwesomeMod
```

### A simple mod
Let's add some content:

`index.html`:
```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>My Awesome Mod</title>
  <style>
    html, body {
      margin: 0;
      user-select: none;
      width: 100%;
      height: 100%;
      background-color: black;
      overflow: hidden;
      color: white;
    }
  </style>
</head>
<body>
  <h1>Hello, world! Click anywhere for a message.</h1>
  <script src="index.js"></script>
</body>
</html>
```
`index.js`:
```js
document.addEventListener('click', (e) => {
    alert("Hello world!");
})
```
A few notes:
- You probably want to make sure the body is styled as `user-select: none;`. This prevents users from dragging to select text on the wallpaper. `overflow: hidden;` is also useful for disabling the scroll bar on the side of the screen.
- In addition, notice how you can reference any local files within your mod folder like `index.js` in your files.
- Lastly, one of Octos' most important features: user input! Currently, all types of mouse events on the wallpaper are supported.

### Testing and debugging
Now we have a nice and simple mod to test out as our wallpaper. There are a few ways to test and debug your mod:
- **In the browser**: this is good for simple mods that don't need to use the Octos API or any other Octos features. Simply go to `file:///C:/path/to/MyAwesomeMod` in your browser to preview and debug your HTML.
- **With the Octos CLI**: this is ideal for actually testing your mod on the wallpaper. This will allow you to use any features like the Octos API in your mod as well.
    ```batch
    octos run MyAwesomeMod
    ```
    Then, you can reload your wallpaper when you make a change:
    ```batch
    octos reload
    ```
    In addition, you can enable DevTools for better debugging. Note that this will open separate DevTools windows for each instance of your mod on each monitor/display.
    ```batch
    octos dev-tools
    ```
    See more about using the [Octos CLI](cli.md)
- **Bundled as a .zip**: see the next section for more details

### Sharing your mod
Octos mods are distributed as .zip archive files. To distribute your mod, zip your whole `MyAwesomeMod` mod folder. You can then share it and others can install it through the Octos app.

### Next steps
- Supercharge your mod with the [Octos API](api.md)
- See some [example mods](https://github.com/underpig1/octos-community/tree/master) for inspiration
- Learn more about the [Octos CLI](cli.md)
- Learn how to [publish your mod](publish.md) to the Octos communtiy
