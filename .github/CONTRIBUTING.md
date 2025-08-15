# Contributing guide
Here you can find helpful resources for building the project from source, submitting issues, and submitting PRs.

## Project structure
- `assets/`: content to be copied to `C:/Program Files/Octos` on install, contains app UI
    - `app/`: main app UI in HTML/CSS/JS
    - `wallpapers/`: default wallpapers to include with Octos on install
    - `img/`: image/icon assets for packaging 
- `src/`: main app source code (written mostly in C++/Win32)
    - `API/`: contains C++-side code for handling Octos API requests/events, also includes Media Controller and Audio components of the API
    - `Bridge/`: for handling and dispatching WebView messages from the main app and wallpaper windows
    - `CLI/`: for handling command line arguments to the executable
    - `Core/`: the core engine for wallpaper windows and the main app window (the actual UI in HTML/CSS/JS is in `assets/`), including code for attaching windows to the desktop
    - `Event/`: for capturing low-level mouse events and forwarding them to the WebView2 Composition Controller (necessary because wallpaper windows don't normally accept mouse input)
    - `js/`: source code for the frontend Octos API (written in JS), allows JS-side to interface with C++-side
    - `Storage/`: wallpaper storage, installation, and other features involving local storage
    - `TrayIcon/`: for minimizing Octos to the system tray and sending tray notifications
    - `Watchdog/`: watchdog cycle for making sure each wallpaper window is in the right place, listens for and addresses display changes
    - `WebView/`: initializing WebView2 environment and windows (uses Composition Controller for wallpaper windows and regular Controller for main UI)
    - `main.cpp`: main UI thread
    - `main.h`: contains globals
- `docs/`: documentation source
- `test/`: safe to ignore, just for testing out minimal examples of features before I officially implement them in the app
- `scripts/`: batch scripts for building and packaging

## Building the app from source
Octos is written in [C++/Win32](https://learn.microsoft.com/en-us/windows/win32/api/) and is mainly powered by [Microsoft Edge WebView2](https://developer.microsoft.com/en-us/Microsoft-edge/webview2/?form=MA13LH) for displaying web content. Building is done with [MSBuild](https://learn.microsoft.com/en-us/visualstudio/msbuild/msbuild?view=vs-2022) from the command line and uses [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview) for package management.

To build the app, you must be using a [Visual Studio Integrated Developer Command Prompt](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022) to correctly set up the environment and necessary binaries. Next, navigate to the `octos/` directory.

In the developer command prompt, install dependencies (as specified in `vcpkg.json`):
```
vcpkg install --triplet x64-windows
vcpkg integrate install
```

Build app:
```
msbuild Octos.vcxproj /p:Configuration=Debug
```

Then, the executable `Octos.exe` will appear in the `build/Debug/` folder.

Note that all content from `assets` must be next to the executable in order for it to run properly. You shouldn't have to worry about this since all content from `assets` will be copied to `build/Debug/` automatically during `msbuild`.

## Packaging the app for distribution
TBD

## Bundling the Octos API
The frontend side of the Octos API lives in `src/js/`. If you make changes to these files, you can bundle the Octos API into an `octos.min.js` file using [rollup.js](https://rollupjs.org/). You need [`npm`](https://www.npmjs.com/) to do this. Rollup config is found in `rollup.config.js`.

First, install dependencies:
```
npm install
```

Then, bundle the Octos API:
```
npm run build-api
```

This should produce `octos.min.js` in the root directory.

If you make changes to the backend side of the API in `src/API/`, you'll need to follow the instructions above for building the app from source in order for these changes to take effect.

## Docs
The Octos docs, including the main site homepage are powered by [Material for MkDocs](https://squidfunk.github.io/mkdocs-material/). The config file for mkdocs can be found in `mkdocs.yml`.

### Building the API Reference
The API reference section of the docs is built using [JsDoc](https://jsdoc.app/) tags in the API source in `src/js/` and [jsdoc-to-markdown](https://github.com/jsdoc2md/jsdoc-to-markdown) to convert those tags to markdown. You need [`npm`](https://www.npmjs.com/) to do this.

First, install dependencies:
```
npm install
```

Then, build the docs:
```
npm run build-docs
```

This will generate the markdown files for the API reference in `docs/reference/`

### Previewing the docs
You need Python and `pip` installed to locally serve and preview the docs. First, install it with `pip`:
```
pip install mkdocs-material
```

Then, install additional MkDocs plugins:
```
pip install -r docs\requirements.txt
```

Then, serve the docs at `localhost:8000` using the mkdocs CLI:
```
mkdocs serve
```

### Building the docs
Docs are automatically built and served with GitHub Actions on push.

## Contributing to the Octos Community
If you made a cool mod and you want to contribute it to the Octos app, read the [publishing guide](https://underpig1.github.io/octos/guides/publish/).

## Pull requests
You are more than welcome to submit changes to the source repository via a pull request.

## Issues
Octos uses GitHub issues to track public bugs. Please make sure the description is clear and you provide exact steps for reproducing the issue.

Feature requests of any size are also more than welcome.

## License
By contributing, you agree that your contributions will be licensed under the LICENSE file in the root directory of this source tree.

## Thank you!
Thanks for visiting! Your contributions are always appreciated, big or small.