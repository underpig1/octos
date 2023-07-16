# FileSystem

Provides a limited interface for reading and writing files.

```js
new FileSystem()
```

## Methods

```js
fs.readdir(path)
```

- `path` string - The path on the user's file system to read

Returns: `Promise<Object[]>` - Resolves with an array of directory objects, each containing `path`, `name`, `isDir` for that directory

Gets an array of all file paths in a given directory. `path` can contain environmental variables such as `%AppData%` and `%UserProfile%`. `FileSystem` will also resolve `%Desktop%` as the user's desktop folder.
<br><br>
```js
fs.open(target)
```

- `target` string - The target file, directory, or URL

Opens the given target in the user's file system. If `target` points to a directory, `FileSystem` will reveal it in Windows FileExplorer. If `target` points to a file, `FileSystem` will attempt to open it in its default application. If `target` is a URL, `FileSystem` will open it in the default web browser.
<br><br>
```js
fs.extractIcon(path)
```

- `path` string - The path from which to extract an icon

Returns: `Promise<string>` - Resolves with a string containing the DataURL of the extracted icon

Extracts an icon as a DataURL from the file/directory at the given path. `path` can contain environmental variables such as `%AppData%` and `%UserProfile%`. `FileSystem` will also resolve `%Desktop%` as the user's desktop folder.
<br><br>
```js
fs.pathExists(path)
```

- `path` string - The path to check

Returns: `Promise<boolean>` - Resolves with `true` if the given path exists or `false` if it does not

Checks if the given path (file or directory) exists in the user's file system. `path` can contain environmental variables such as `%AppData%` and `%UserProfile%`. `FileSystem` will also resolve `%Desktop%` as the user's desktop folder.
<br><br>
```js
fs.isDirectory(path)
```

- `path` string - The path to check

Returns: `Promise<boolean>` - Resolves with `true` if the given path is a directory or `false` if it is not

Checks if the given path is a directory. `path` can contain environmental variables such as `%AppData%` and `%UserProfile%`. `FileSystem` will also resolve `%Desktop%` as the user's desktop folder.

## Examples

Creates clickable icons as a substitute for the user's desktop:

```js
fs.readdir("%Desktop%").then(async (dirs) => {
    for (var dir of dirs) {
        var img = document.createElement("img");
        img.onclick = () => fs.open(dir.path);
        img.src = await fs.extractIcon(dir.path);
        img.title = dir.name;
        document.body.appendChild(img);
    }
});
```