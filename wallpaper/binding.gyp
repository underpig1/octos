{
    "targets": [
        {
            "target_name": "wallpaper",
            "sources": [
                "./bind.cpp",
                "./input.cpp",
                "./media.cpp"
            ],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        }
    ]
}
