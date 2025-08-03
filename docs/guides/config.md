# `octos.json` Reference

Each mod folder can optionally include an `octos.json` file, which includes metadata, the mod's entry point, a preview image for the explore page, and user options.

## Example
`octos.json`:
```json
{
  "name": "My Awesome Mod",
  "author": "Me",
  "description": "A dynamic live wallpaper with interactive elements.",
  "image": "assets/image.png", // path to card image to display in Octos app
  "preview": "assets/preview.gif", // path to preview image to display in explore page
  "entry": "index.html", // entry point, path to local HTML file or external webpage
  "options": { // user options, configurable in Octos app
    "enable-effect": {
      "type": "checkbox",
      "label": "Enable effect?",
      "value": true
    },
    "speed": {
      "type": "range",
      "label": "Speed",
      "min": 1,
      "max": 10,
      "value": 5
    }
  }
}
```

## Fields

### `author`
**Type:** `string`  
The author's name.

### `name`
**Type:** `string`  
Mod's display name.

### `description`
**Type:** `string`  
A short description of the mod's purpose or features.

### `image`
**Type:** `string`  
Path within the mod folder to the card image to be shown in the Octos app. Typically a stylistic/aesthetic image that doesn't necessarily represent the mod as it appears on the wallpaper. Use the `preview` field to specify a more accurate image/animation.

### `preview`
**Type:** `string`
Path within the mod folder to the preview image/animated gif to be shown in the Octos explore page. Typically more like an accurate representation of how the mod will appear as a wallpaper.

### `entry`
**Type:** `string`  
The main HTML entry point that launches the wallpaper. Can either be a local HTML file or an external webpage.

### `options`
**Type:** `object`  
A JSON object that defines one or more **customizable settings** that can be changed by users of your mod in the Octos app:

<img src="../../img/user-options.png" width="150px" aria-hidden />

These settings, along with input change event listeners, can be accessed in JS with the [`UserOptions` class](../reference/useroptions.md) of the [Octos API](api.md).

Each property inside `options` defines a single input control.

The keys of the `options` object represent IDs that can be accessed with the `UserOptions` API.

#### `option` object properties:
| Property | Type   | Description |
|----------|--------|-------------|
| `type`   | `string` | The type of input. Supported: `checkbox`, `range`, `file`, `description`, `select`, `color-picker`, `dropdown`, `number`, `text` |
| `label`  | `string` | Label text shown to the user (required). |
| `value`  | `any`    | The default value for the input. |
| `min`    | `number` | *(range/number only)* Minimum value (optional). |
| `max`    | `number` | *(range/number only)* Maximum value (optional). |
| `step`   | `number` | *(range/number only)* Step size (optional). |
| `options` | `array` | *(select only)* List of possible options for select input. |
| `onchange` | `string` | JavaScript function to execute on change. Passes `value` of the changed input to the function. Ex. `"onchange": "(value) => alert(value)"`. |
| `onload` | `string` | JavaScript function to execute on load. Passes `value` of the loaded input to the function. Ex. `"onload": "(value) => alert(value)"`. |

#### Notes:
- An option of `"type": "description"` only renders its `label` property.
- An option must include `label` to render.
- `options` maps directly to standard HTML input elements.
- Add as many options as you like — they will be rendered dynamically in the app.

#### Example `options`

```json
"options": {
  "enableParticles": {
    "type": "checkbox",
    "label": "Enable Particles",
    "value": true
  },
  "particleSize": {
    "type": "range",
    "label": "Particle Size",
    "min": 1,
    "max": 20,
    "step": 1,
    "value": 5
  },
  "theme": {
    "type": "select",
    "label": "Theme",
    "options": ["Light", "Dark", "Auto"],
    "value": "light"
  }
}
```

## Notes
- All paths should be relative to the mod folder root location. Paths may also be external links.

## Example Folder Structure
```
MyAwesomeMod/
├── index.html
├── octos.json
├── assets/
│   └── preview.png
│   └── image.png
└── js/
│   └── index.js
└── css/
    └── style.css
```
