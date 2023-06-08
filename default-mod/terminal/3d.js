var vec3 = (x, y, z) => {
    return { x, y, z };
}
var vec2 = (x, y) => {
    return { x, y };
}

var c = vec3(0, 0, -100);
var r = vec3(0, 0, 0);
var e = vec3(0, 0, 100);

function project3d(a) {
    var x = a.x - c.x;
    var y = a.y - c.y;
    var z = a.z - c.z;
    var dx = Math.cos(r.y) * (Math.sin(r.z) * y + Math.cos(r.z) * x) - Math.sin(r.y) * z;
    var dy = Math.sin(r.x) * (Math.cos(r.y) * z + Math.sin(r.y) * (Math.sin(r.z) * y + Math.cos(r.z) * x)) + Math.cos(r.x) * (Math.cos(r.z) * y - Math.sin(r.z) * x);
    var dz = Math.cos(r.x) * (Math.cos(r.y) * z + Math.sin(r.y) * (Math.sin(r.z) * y + Math.cos(r.z) * x)) - Math.sin(r.x) * (Math.cos(r.z) * y - Math.sin(r.z) * x);
    var px = e.z / dz * dx + e.x;
    var py = e.z / dz * dy + e.y;
    return vec2(px, py);
}

function line3d(a, b) {
    line2d(project3d(a), project3d(b));
}

function face3d(f) {
    for (var i = 0; i < f.length - 1; i++) line3d(f[i], f[i + 1]);
    line3d(f[i], f[0]);
}

function mesh3d(m) {
    for (var i = 0; i < m.length; i++) face3d(m[i]);
}

function rotate3d(m, t, a = vec3(0, 0, 0)) {
    var ca = Math.cos(t.x), sa = Math.sin(t.x), cb = Math.cos(t.y), sb = Math.sin(t.y), cc = Math.cos(t.z), sc = Math.sin(t.z);
    for (var f of m) {
        for (var p of f) {
            var px = p.x - a.x;
            var py = p.y - a.y;
            var pz = p.z - a.z;
            p.x = ca * cb * px + (ca * sb * sc - sa * cc) * py + (ca * sb * cc + sa * sc) * pz + a.x;
            p.y = sa * cb * px + (sa * sb * sc + ca * cc) * py + (sa * sb * cc - ca * sc) * pz + a.y;
            p.z = -sb * px + cb * sc * py + cb * cc * pz + a.z;
        }
    }
}

function centerOfMass(m) {
    var s = vec3(0, 0, 0);
    var fm = m.flat();
    for (var i = 0; i < fm.length; ++i) {
        s.x += fm[i].x;
        s.y += fm[i].y;
        s.z += fm[i].z;
    }
    return vec3(s.x / i, s.y / i, s.z / i);
}

function centerByMass(m) {
    var ct = centerOfMass(m);
    for (var f of m) {
        for (var p of f) {
            p.x -= ct.x;
            p.y -= ct.y;
            p.z -= ct.z;
        }
    }
}

function distanceFromCamera(m) {
    var ct = centerOfMass(m);
    return Math.sqrt((c.x - ct.x) ** 2 + (c.y - ct.y) ** 2 + (c.z - ct.z) ** 2);
}

var cube = (scale) => [
    [[-1, -1, -1], [-1, -1, 1], [-1, 1, 1], [-1, 1, -1]],
    [[1, -1, -1], [1, -1, 1], [1, 1, 1], [1, 1, -1]],
    [[-1, 1, -1], [-1, 1, 1], [1, 1, 1], [1, 1, -1]],
    [[-1, -1, -1], [-1, -1, 1], [1, -1, 1], [1, -1, -1]],
    [[-1, -1, -1], [1, -1, -1], [1, 1, -1], [-1, 1, -1]],
    [[-1, -1, 1], [-1, 1, 1], [1, 1, 1], [1, -1, 1]]
].map((c) => c.map((k) => vec3(...k.map((j) => j * scale))));

var m = cube(20);
setInterval(() => {
    ctx.clearRect(0, 0, 1000, 1000);
    rotate3d(m, vec3(0.01, 0.01, 0.01));
    mesh3d(m);
}, 10);

function encloses(point, shape) {
    var intersect = false;
    for (var i = 0, j = shape.length - 1; i < shape.length; j = i++) {
        if (((shape[i].y > point.y) != (shape[j].y > point.y)) && (point.x < (shape[j].x - shape[i].x) * (point.y - shape[i].y) / (shape[j].y - shape[i].y) + shape[i].x)) intersect = !intersect;
    }
    return intersect;
}