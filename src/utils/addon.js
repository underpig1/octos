const addon = require("../../wallpaper/build/Release/wallpaper");

module.exports.attach = addon.attach;
module.exports.detach = addon.detach;
module.exports.mousePosition = addon.mposition;
module.exports.leftMousePressed = addon.lmpressed;
module.exports.inForeground = addon.infg;
module.exports.setTaskbar = addon.settb;
module.exports.inForeground = addon.infg;

module.exports.middleMousePressed = addon.mmpressed;
module.exports.rightMousePressed = addon.rmpressed;
module.exports.keyboard = addon.keyboard;
module.exports.sendMediaEvent = addon.sendmedia;
module.exports.fgTitle = addon.title;

module.exports.trackTitle = addon.ttitle;
module.exports.trackArtist = addon.tartist;
module.exports.trackTimeline = addon.ttime;
module.exports.playbackStatus = addon.tstatus;