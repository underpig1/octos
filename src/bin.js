#!/usr/bin/env node

if (!process.argv.includes("init") && !process.argv.includes("build")) require(require("path").join(__dirname, "main.js"));
parseArgs();

function parseArgs() {
    const yargs = require("yargs/yargs");
    const { hideBin } = require("yargs/helpers");

    yargs(hideBin(process.argv)).command("init [title] [path]", "create a new mod in the given directory", (yargs) => {
        return yargs.positional("title", {
            describe: "title of mod",
            default: "very cool mod"
        }).positional("path", {
            describe: "directory in which to create a new mod folder",
            default: process.cwd()
        });
    }, (argv) => {
        initMod(argv.path, argv.title);
    }).command("run [path]", "run mod at the given path", (yargs) => {
        return yargs.positional("path", {
            describe: "path to the file or directory containing the target mod",
            default: process.cwd()
        });
    }, (argv) => {
        runMod(argv.path);
        if (argv.debug) {
            cmenu.getMenuItemById("toggledev").checked = true;
            win.webContents.openDevTools({ mode: "detach" });
        }
        console.log("Running mod at " + argv.path);
    }).option("devtools", {
        alias: "d",
        type: "boolean",
        description: "run with Chrome DevTools enabled"
    }).command("build [path]", "build mod at the given path into a .omod", (yargs) => {
        return yargs.positional("path", {
            describe: "path to the file or directory containing the target mod",
            default: process.cwd()
        });
    }, (argv) => {
        buildMod(argv.path);
    }).parse();
}

function initMod(working, title) {
    const fs = require("fs-extra");
    var dir = path.join(working, title);
    try {
        fs.mkdirSync(dir);
        fs.copySync(path.resolve(__dirname, "../example/init-mod"), dir, { overwrite: false });
        var filepath = path.join(dir, "mod.json");
        var config = require(filepath);
        config.title = title;
        config.author = author;
        config.description = description;
        fs.writeFile(filepath, JSON.stringify(config, null, 2));
        console.log("Created new mod folder at " + dir);
        exit();
    }
    catch {
        console.error("Provided directory does not exist");
    }
}


function buildMod(dir) {
    const archiver = require("archiver");
    const fs = require("fs");
    const archive = archiver("zip", { zlib: { level: 9 } });
    var outpath = dir + ".omod";
    const stream = fs.createWriteStream(outpath);

    console.log("Building mod...");
    stream.on("close", () => {
        console.log("Mod created at " + outpath);
        exit();
    });
    archive.directory(dir, false).on("error", (err) => {
        console.error("There was an issue building your mod. See documentation for manual build.");
    }).pipe(stream);
    archive.finalize();
}