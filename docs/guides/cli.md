# Using the Octos CLI

The Octos command line interface (CLI) allows you to easily test and debug your custom mods from the command line.

## Setup
Depending on your installation method, you may have had the option to add `octos` to your system PATH environmental variable. This allows you to run the `octos` command from any command line without having to specify the entire path.

You can see if `octos` is added to your path simply by running it in your command line:
```
octos
```

If it opens the app, you're all set. You can now use Octos commands from anywhere.

If not, you can either specify the full path to the `octos.exe` app when you run it from the command line (likely installed at `C:\Program Files\Octos\octos.exe`, although the location depends on your installation method), or you can manually add it to PATH.

## Usage

```batch
octos <command> [options]
```

Read on to learn about specific commands for running, testing, and debugging your mods.

## Commands

### `run`

Run the mod in the specified folder on the wallpaper for testing/debugging. This allows you to test your mod with full Octos-specific functionality, like [user options](options.md) and the [Octos API](api.md). When you make changes to your mod while testing, you can re-run this command to refresh/update your mod on the wallpaper and in the app.

When you run a mod with this command, it will temporarily appear as an installed mod in your library in the Octos app. This allows you to preview how your mod will look in the app and lets you test out changes to [user options](options.md) and [`octos.json`](config.md). Note that if you close the app your mod will no longer appear in your library the next time you open it, and you will need to re-run the command.

Running a mod this way will also give you full access to the [Octos API](api.md).

```batch
octos run <folderPath> [--dev-tools]
```

**Options:**
- `<folderPath>` — *(required)* Path to the mod folder. Must exist and be a valid directory.
- `--dev-tools` — *(optional)* Open developer tools for all wallpaper windows after launch. Note that this will open a separate DevTools window for each monitor that your mod is running on.
<!-- - `--auto` — *(optional)* Enable automatic behavior. -->

The given folderPath must be a valid mod folder, which means it must have a valid HTML `entry` point. This can be specified explicitly in the optional [`octos.json`](config.md) file in the folder's root (pointing to a local HTML file or external webpage), or if unspecified, Octos will look for a valid HTML file to load.

**Example:**

```batch
octos run ./MyAwesomeMod --dev-tools
```

### `reload`

Reload all wallpaper windows. This is equivalent to refreshing in the browser. While testing with `run`, use `reload` to quickly update your mod's web content on the wallpaper after you make a change to HTML/CSS/JS files in the folder. Note that this command will only reload your mod's web content and won't react to updates to your `octos.json` file (for that, just use `run` again).

```batch
octos reload
```

### `new`

Create a new mod folder with starting `index.html` and `octos.json` files.

```batch
octos new <name>
```

**Example:**

```batch
octos new MyAwesomeMod
```

Running the previous command will create the following folder structure in the current working directory:

```
MyAwesomeMod/
├── index.html
└── octos.json
```

`<name>` can also point to an existing folder path in which to create the starting files:

```batch
mkdir MyAwesomeMod
cd MyAwesomeMod
octos new .
```

This is functionally the same as the previous example.

### `dev-tools`

Open developer tools for all wallpaper windows. This is the same as using `run` with the `--dev-tools` flag. This command isn't limited to use for debugging your own mods; you can also use it to peek inside and debug other mods that you have installed.

```batch
octos dev-tools
```

Note that this will open a separate DevTools window for each monitor that is running an Octos mod.