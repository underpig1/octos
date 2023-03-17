const addon = require("bindings")("wallpaper");

exports.attach = (win) => addon.attach(win.getNativeWindowHandle());
exports.detach = (win) => addon.detach(win.getNativeWindowHandle());
exports.mousePosition = addon.mposition;
exports.leftMousePressed = addon.lmpressed;
exports.inForeground = addon.infg;
exports.setTaskbar = addon.settb;
exports.inForeground = addon.infg;

exports.middleMousePressed = addon.mmpressed;
exports.rightMousePressed = addon.rmpressed;
exports.keyboard = addon.keyboard;