# FileDialog

Create and control file dialogs to request filepaths or directories from the user.

```js
new FileDialog([options])
```

- `options` Object (optional)
    - `extensions` string[] (optional) - File extensions that the file dialog accepts
    - `type` string (optional) - The type of input that the file dialog accepts
        - `"directory"` - Prompt user for directory
        - `"file"` - Prompt user for file

## Events

### `error`

Returns: `Exception`

### `finish`

Returns: `event` containing `filepath` as string

## Methods

```js
dialog.request()
```

Requests a filepath from a user.

Returns: `FileDialog` object

## Examples

Prompt the user for an image file:

```js
new FileDialog({ extensions: [".png", ".jpg", ".jpeg"], type: "file" })
    .request()
    .on("error", (err) => console.error(err))
    .on("finish", (e) => console.log("Got file at " + e.filepath))
```

Prompt the user for a directory:

```js
new FileDialog({ type: "directory" })
    .request()
    .on("error", (err) => console.error(err))
    .on("finish", (e) => console.log("Got directory at " + e.filepath))
```