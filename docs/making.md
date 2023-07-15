# Making a mod

Getting started with developing for Octos is easy. This guide will get you started with making your own wallpapers fast.

### With the Developer Menu

### With the CLI
In the command line, run `octos init` to make a new mod

# File structure
```
my-mod
| index.html
| mod.json
```
Store any files you need for your mod in single folder containing a `mod.json` file. Change the `mod.json` to set mod settings and configure mod behavior.

A basic `mod.json` might look something like this:
```json
{
    "name": "My custom mod",
    "description": "A very cool mod I made that does very cool things",
    "entry": "index.html"
}
```
Change `"entry"` to the relative path to the HTML file or URL to a webpage that is rendered when a user starts up your mod.

[Learn more about mod.json...](https://underpig1.github.io/octos/docs/?t=mod-json)

# Example
See an example of a simple wallpaper that shows some text in the middle of the screen.

`index.html`:
```html
<html>
<head>
    <style>
        html, body {
            height: 100%;
            background-color: white;
            text-align: center;
            line-height: 100vh;
        }
    </style>
</head>
<body>
    <h1>My very cool mod</h1>
</body>
</html>
```
`mod.json`:
```json
{
    "name": "My custom mod",
    "description": "A very cool mod I made that does very cool things",
    "entry": "index.html"
}
```

# Native API
Use the Octos API to power up your wallpaper with any of the following:
- Get playback info
- Media/playback controls
- Read and write to local storage
- Access file system
- Read and write user preferences
- Access system information
- And more...

To get started, include the following script tag in your HTML file:
```html
<script src="https://unpkg.com/octos@latest/octos.js"></script>
```
Or directly download the [octos.js](https://raw.githubusercontent.com/underpig1/octos/master/octos.js) file and include it in your mod folder:
```html
<script src="octos.js"></script>
```

[Learn more with the API Docs...](https://underpig1.github.io/octos/docs/?t=using-api)

# Testing
Test your mod to see how it runs without having to build/package it.

### With the Developer Menu

### With the CLI
In the command line, run `octos run path/to/mod/folder` to test out your mod.

[Learn more about testing...](https://underpig1.github.io/octos/docs/?t=testing)

# Package and share

Once you finalize your mod, you'll need to bundle it into a `.omod` file for others to download and install.

### With the Developer Menu

### With the CLI
In the command line, run `octos build path/to/mod/folder` to bundle it into a distributable `.omod` file.

[Learn more about sharing your mod...](https://underpig1.github.io/octos/docs/?t=building)

# Publish

[See the pulishing guide](https://underpig1.github.io/octos/docs/?t=publish) for more information about sharing your mod on the platform for others to download through the app. Contributions are always appreciated!