# UserPreferences

Access and define user preferences for your mod which can be changed by the user in the Octos app.

```js
new UserPreferences()
```

User preferences allow a user to set certain mod-specific settings and customize your mod from the Octos app. This API class allows you to read and write the values for these inputs as well as listen to changes in these inputs.

You can define your mod's user preferences in the `mod.json` file under the `prefs` field. [Learn more](?t=mod-json#prefs).

## Events

### `change`

Returns: `event` containing `prefs` as an object, which stores the values for each input as defined in your mod's `mod.json` file.

Emitted when the user changes any mod-specific inputs. It is also always emitted when the mod starts up to give information about the current state of the user's preferences.

## Methods

```js
prefs.read([id])
```

- `id` string - The ID of the input you want to read

Returns: `Promise<string>` - Resolves as a string containing the input's value (set by the user)

Sends a request to access the value of the input with the given ID. The returned values are in the string format, so if you want an integer value you'll have to use `parseInt` to cast it to an integer.
<br><br>
```js
prefs.write([id, value])
```

- `id` string - The ID of the input you want to set
- `value` - The value of the input you want to set

Sends a request to set the value of the input with the given ID.

## Examples

Sets the background color to a user-defined value:

In your `mod.json`:
```json
{
    "prefs": {
        "background-color": {
            "type": "color",
            "value": "#ffffff",
            "label": "Background color"
        },
        "dark-mode": {
            "type": "checkbox",
            "value": true,
            "label": "Enable dark mode?"
        }
    }
    ...
}
```

In your HTML file:
```js
const prefs = new UserPreferences();

prefs.on("change", (e) => {
    if (e.prefs["dark-mode"]) {
        document.body.style.backgroundColor = "black";
    }
    else {
        document.body.style.backgroundColor = e.prefs["background-color"]
    }
})
```