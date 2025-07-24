# Using the Octos CLI

The Octos command line interface (CLI) allows you to easily test and debug your custom mods from the command line.

## Usage

```batch
octos <command> [options]
```

## Commands

### `run`

Run the wallpaper in the specified folder.

```batch
octos run [--dev-tools] <folderPath>
```

**Options:**
<!-- - `--auto` — *(optional)* Enable automatic behavior. -->
- `--dev-tools` — *(optional)* Open developer tools for all wallpaper windows after launch.
- `<folderPath>` — *(required)* Path to the mod folder. Must exist and be a valid directory.

**Example:**

```batch
octos run --dev-tools ./MyAwesomeMod
```

### `reload`

Reload all wallpaper windows.

```batch
octos reload
```

### `new`

Create a new wallpaper with a starting `index.html` and `octos.json` in the specified folder.

```batch
octos new <folderPath>
```

**Example:**

```batch
mkdir MyAwesomeMod
octos new MyAwesomeMod
```

### `dev-tools`

Manually open developer tools for all wallpaper windows.

```batch
octos dev-tools
```

### Notes
- If `<folderPath>` is not valid or missing where required, the command will do nothing.