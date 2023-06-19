module.exports = {
    packagerConfig: {
        icon: "./img/octos.ico"
    },
    rebuildConfig: {},
    makers: [
        {
            name: "@electron-forge/maker-wix",
            config: {
                language: 1033,
                manufacturer: "Octos"
            }
        }
    ],
    publishers: [
        {
            name: "@electron-forge/publisher-github",
            config: {
                repository: {
                    owner: "underpig1",
                    name: "octos"
                },
                prerelease: true
            },
            config: {
                icon: "./img/octos.ico",
            }
        }
    ],
    hooks: {
        packageAfterCopy: [
            async (buildPath, electronVersion, platform, arch) => {
                require("child_process").execSync(`REG ADD HKCU\\Software\\Classes\\.omod /ve /d "octos.OctosFile" /f
REG ADD HKCU\\Software\\Classes\\octos.OctosFile /ve /d "Octos File" /f
REG ADD HKCU\\Software\\Classes\\octos.OctosFile\\DefaultIcon /ve /d "${path.join(__dirname, "img/omod.ico")}" /f
REG ADD HKCU\\Software\\Classes\\octos.OctosFile\\Shell\\Open\\Command /ve /d "\"${path.join(buildPath, "octos.exe")}\" add \"%1\"" /f
SET PATH=%PATH%;${path.join(buildPath, "octos.exe")}`, { windowsHide: true });
            },
        ],
    },
};
