module.exports = {
    packagerConfig: {
        icon: "./img/octos.ico",
        ignore: [
            "./docs",
            "./img/gallery",
            "./octos-community",
            "./wallpaper/node_modules",
            "./node_modules",
            "./web"
        ]
    },
    rebuildConfig: {},
    makers: [
        {
            name: "@electron-forge/maker-zip",
        },
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
    ]
};
