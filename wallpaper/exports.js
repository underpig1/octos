const addon = require("bindings")("wallpaper");

exports.attach = (handle) => addon.attach(handle);
exports.detach = (handle) => addon.detach(handle);
exports.mousePosition = addon.mposition;
exports.leftMousePressed = addon.lmpressed;
exports.inForeground = addon.infg;
exports.setTaskbar = addon.settb;
exports.inForeground = addon.infg;

exports.middleMousePressed = addon.mmpressed;
exports.rightMousePressed = addon.rmpressed;
exports.keyboard = addon.keyboard;
exports.sendMediaEvent = addon.sendmedia;
exports.fgTitle = addon.title;

exports.trackTitle = addon.ttitle;
exports.trackArtist = addon.tartist;
exports.trackTimeline = addon.ttime;
exports.playbackStatus = addon.tstatus;