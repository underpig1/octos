# Making your own mod

In Octos, custom HTML wallpapers are called mods. Creating a mod is super easy in Octos. Let's work through a simple example.

### Getting started
Start by making a folder that looks like this:
```
MyAwesomeMod/
├── index.html
├── octos.json
```
In this folder, you can include assets, JS libraries, CSS files, icons, scripts, and basically anything else that your mod needs to run.

The `octos.json` file is completely optional but it allows you to specify a lot of details for when you want to distribute your mod. These include fields like name, author, main page, user options, preview images, and a lot more. Your mod will work just fine without it, but it's good practice to add.

For now, let's put this in our `octos.json`:
```json
{
    "name": "My Awesome Mod",
    "description": "This is an awesome mod.",
    "entry": "index.html"
}
```
The `entry` field basically tells Octos where your mod's main webpage is located. If you don't put specify this field, Octos will automatically look for `index.html` or any other HTML file in your mod folder, so you probably don't need to worry about this for now, but it's good practice if you later have a bunch of HTML files in the folder. `entry` can also be a link to an external webpage!

Recall that, just like any other webpage, we can add any files we need to our mod folder. Let's add an `index.js` to our mod for some JavaScript functionality:
```
MyAwesomeMod/
├── index.html
├── index.js
├── octos.json
```

Alternatively, you can generate a starting mod folder, which includes minimal `index.html` and `octos.json` files with the [Octos command line interface (CLI)](cli.md):
```batch
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

- **In the browser**: this is a good start for simple mods that don't need to use the Octos API or any other Octos features. Simply go to `file:///C:/path/to/MyAwesomeMod` in your browser to preview and debug your HTML. You could also [setup a local server](https://developer.mozilla.org/en-US/docs/Learn_web_development/Howto/Tools_and_setup/set_up_a_local_testing_server) at `localhost` for more advanced testing.  

- **With the Octos CLI**: this is ideal for actually testing your mod on the wallpaper. This will allow you to use any features like the Octos API and test out user options in your mod as well. Simply call `octos run path/to/mod/folder` in your command line to test your mod on the wallpaper.  
    ```batch
    octos run ./MyAwesomeMod
    ```
    Then, you can reload your wallpaper when you make a change:
    ```batch
    octos reload
    ```
    In addition, you can enable DevTools for better debugging.
    ```batch
    octos dev-tools
    ```
    Learn more about using the [Octos CLI](cli.md). It's pretty powerful for testing out and debugging mods.  

- **Bundled as a .zip**: see the next section for more details

### Sharing your mod
Octos mods are distributed as .zip archive files. To distribute your mod, simply zip your whole `MyAwesomeMod` mod folder. You can then share it and others can install it through the Octos app by clicking "Install mod from .zip" and then clicking on MyAwesomeMod.zip.

### Next steps
- Supercharge your mod with the [Octos API](api.md)  

- See some [example mods](https://github.com/underpig1/octos-community/tree/master) for inspiration  

- Learn more about the [Octos CLI](cli.md) for testing and debuugging  

- Learn how to [publish your mod](publish.md) to the Octos community  
