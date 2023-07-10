module.exports = {
    packagerConfig: {
        icon: "./img/octos.ico",
        ignore: [
            "./docs",
            "./img/gallery"
        ]
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
    ]
};
