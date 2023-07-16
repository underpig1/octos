# mod.json

This guide contains details about how to configure your mod to specify metadata, mod settings, and user-controllable preferences.

## Introduction

Every mod folder contains a `mod.json` file in the root directory.

A basic `mod.json` might look something like this:
```json
{
    "name": "My custom mod",
    "description": "A very cool mod I made that does very cool things",
    "entry": "index.html"
}
```

Octos uses this file to identify important information about your mod. A `mod.json` file is optional. If a mod does not contain a `mod.json` file or if the field is not specified, Octos will use the name of the folder containing the mod as the mod's name in place of the `name` field. It will also assume a default entry of `index.html` if one is not specified.

## name

The name of your mod. It is important to give your mod a unique name or else Octos may have trouble distinguishing between two mods with the same name and may overwrite a previous mod with the same name. Avoid using special characters in the name.

Example:
```json
"name": "My cool mod"
```

## description

The description of your mod. This description will appear when you view the mod in the Octos app.

Example:
```json
"description": "This mod does very cool things"
```

## version

The current version of your mod. This field is completely optional and is only for your or your user's convenience.

Example:
```json
"version": "1.0.0"
```

## entry

The path (relative to the root directory of the mod folder) to the entry HTML file for your mod. This is the HTML file that will be rendered when a user first starts up your mod. Entry can also be a url to a webpage.

The rendered file may change as the mod is active (by changing the `location.href`), but Octos uses this field as the entry point for when the user starts up your mod.

Example:
```json
"entry": "index.html"
```

## image

The path (relative to the root directory of the mod folder) to the preview image file for your mod. This image is used when rendering the mod preview in the Octos app on the Explore page and on the Modules page. This field is completely optional.

Example:
```json
"image": "img/preview.png"
```

## author

The author of your mod. This field is completely optional.

Example:
```json
"author": "me"
```

## source

A URL pointing to the location of the source code for your mod. Use this to link your mod to an external page like GitHub. This field is completely optional.

Example:
```json
"source": "https://github.com/underpig1/my-awesome-mod"
```

## options

Specify mod-specific options. Use it to configure event permissions.

```json
"options": {
    "events": {
        "mouse": true,
        "keyboard": false,
        "media": true
    }
}
```

Setting any events to `false` will disable your mod's access to that event. For example, disabling `mouse` events makes your mod unable to recieve mouse events through JS event listeners. Disabling unnecessary events can improve mod performance. All events are enabled by default.

## prefs

An object containing input fields that can be set by the user in the Octos app. The keys of the `prefs` object represent the input IDs that can be used to index an input's value with the [UserPreferences](?t=user-preferences) class of the Octos API. User preferences are useful for giving the user control over certain settings in your mod, like changing the background color, setting a theme, and more.

Example:
```json
"prefs": {
    "my-input-slider": {
        "type": "range",
        "value": "10",
        "min": 5,
        "max": 20,
        "label": "My input slider"
    },
    ...
}
```
Use an input's ID (in this case, `my-input-slider`) to access its value with the Octos API.

`value`: the default value of the input

`label`: the label that accompanies the input

`type`: the input type. It can be any one of the following:
- `color`: a basic color input where `value` is a color hex code
- `file`: a file selector where `value` is a path
- `text`: a text input where `value` is a string
- `checkbox`: a checkbox where `value` is a boolean
- `description`: a standalone `label` without any associated input
- `number`: a number input where `value` is a number
    - `min`: the minimum value of the input
    - `max`: the maximum value of the input
- `range`: a range slider where `value` is an integer
    - `min`: the minimum value of the slider
    - `max`: the maximum value of the slider
    - `step`: the interval between each value
- `select`: a dropdown menu where `value` is the name of one of its options
    - `options`: an array of strings representing the display name of each of the options

`select` example:
```json
"my-select-input": {
    "type": "select",
    "value": "Mercedes",
    "label": "Select a car",
    "options": [
        "Mercedes",
        "Prius",
        "Toyota"
    ]
}
```

