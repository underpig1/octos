# Using the CLI

Octos provides an easy-to-use command line interface with tools for developers.

## CLI reference

Use the `init` command to intialize a mod with the given name at the given path (defaults to working directory).

```text
octos init [name] [path]
```

Use the `run` command to run the mod at the given path without adding it to the user's library. Add the optional `--devtools` flag to enable the debug console.

```text
octos run [path]
```

Use the `build` command to package the mod folder at the given path into the .omod distributable file format.

```text
octos build [path]
```

Use the `add` command to add the mod folder or .omod file at the given path into your installed mod library.

```text
octos add [path]
```