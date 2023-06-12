const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;
canvas.style.width = "100vw";
canvas.style.height = "100vh";

const vec3 = (x, y, z) => {
    return { x, y, z };
}
const vec2 = (x, y) => {
    return { x, y };
}
const dot = (a, b) => a.x*b.x + a.y*b.y + a.z*b.z;
const scale = (a, s) => vec3(a.x*s, a.y*s, a.z*s);
const add = (a, b) => vec3(a.x + b.x, a.y + b.y, a.z + b.z);
const sub = (a, b) => vec3(a.x - b.x, a.y - b.y, a.z - b.z);
const cross = (a, b) => vec3(a.y*b.z - b.y*a.z, a.z*b.x - a.x*b.z, a.x*b.y - b.x*a.y);
const magnitude = (a) => Math.sqrt(a.x ** 2 + a.y ** 2 + a.z ** 2);

var c = vec3(0, 0, -100);
var r = vec3(0, 0, 0);
var e = vec3(0, 0, 1000);

function project3(a) {
    var x = a.x - c.x;
    var y = a.y - c.y;
    var z = a.z - c.z;
    var dx = Math.cos(r.y)*(Math.sin(r.z)*y + Math.cos(r.z)*x) - Math.sin(r.y)*z;
    var dy = Math.sin(r.x)*(Math.cos(r.y)*z + Math.sin(r.y)*(Math.sin(r.z)*y + Math.cos(r.z)*x)) + Math.cos(r.x)*(Math.cos(r.z)*y - Math.sin(r.z)*x);
    var dz = Math.cos(r.x)*(Math.cos(r.y)*z + Math.sin(r.y)*(Math.sin(r.z)*y + Math.cos(r.z)*x)) - Math.sin(r.x)*(Math.cos(r.z)*y - Math.sin(r.z)*x);
    var px = e.z/dz*dx + e.x;
    var py = e.z/dz*dy + e.y;
    return vec2(px, py);
}

function rotate3(m, t, a = vec3(0, 0, 0)) {
    var ca = Math.cos(t.x), sa = Math.sin(t.x), cb = Math.cos(t.y), sb = Math.sin(t.y), cc = Math.cos(t.z), sc = Math.sin(t.z);
    for (var f of m) {
        for (var p of f) {
            var px = p.x - a.x;
            var py = p.y - a.y;
            var pz = p.z - a.z;
            p.x = ca*cb*px + (ca*sb*sc - sa*cc)*py + (ca*sb*cc + sa*sc)*pz + a.x;
            p.y = sa*cb*px + (sa*sb*sc + ca*cc)*py + (sa*sb*cc - ca*sc)*pz + a.y;
            p.z = -sb*px + cb*sc*py + cb*cc*pz + a.z;
        }
    }
}

function cameraNormal() {
    var n = vec3(-Math.sin(r.y), Math.sin(r.x)*Math.cos(r.y), -Math.cos(r.x)*Math.cos(r.y));
    return scale(n, 1/magnitude(n));
}

function distanceFromCamera(p) {
    var n = cameraNormal();
    return Math.abs(dot(n, sub(p, c)));
}

function depthInterpolator(face) {
    var tf = face.slice(0, 3);
    var nface = cross(sub(tf[0], tf[1]), sub(tf[0], tf[2]));
    var dface = -nface.x*tf[0].x - nface.y*tf[0].y - nface.z*tf[0].z;
    return (p) => -(nface.x*p.x + nface.y*p.y + dface)/nface.z;
}

function projectFaceWithDepth(f) {
    var face = [];
    for (var p of f) {
        var projected = project3(p);
        var dist = distanceFromCamera(p);
        face.push(vec3(projected.x, projected.y, dist));
    }
    return face;
}

function projectFace(face) {
    return face.map((c) => project3(c));
}

function arrayToMesh(arr, scale = 1) {
    return arr.map((c) => c.map((k) => vec3(...k.map((j) => j*scale))));
}

function sphereArray(radius = 5, segments = 5) {
    var segsize = radius/segments*2;
    var pcloud = {};
    for (var h = -radius; h <= radius; h += segsize) {
        var r = radius*Math.sqrt(1 - (h/radius) ** 2);
        for (var t = 0, tt = 0; t < 2*Math.PI; t += 2*Math.PI/segments, tt++) {
            var x = r*Math.cos(t);
            var y = r*Math.sin(t);
            if (!pcloud.hasOwnProperty(h)) pcloud[h] = {};
            pcloud[h][tt] = [x, y, h];
        }
    }
    var faces = [];
    for (var h = -radius + segsize; h <= radius; h += segsize) {
        for (var tt = 1; tt < segments; tt++) {
            try {
                faces.push([pcloud[h - segsize][tt - 1], pcloud[h - segsize][tt], pcloud[h][tt], pcloud[h][tt - 1]]);
            }
            catch { }
        }
        faces.push([pcloud[h - segsize][tt - 1], pcloud[h - segsize][0], pcloud[h][0], pcloud[h][tt - 1]]);
    }
    return faces;
}

function triangulateFaces(arr) {
    var triarr = [];
    for (var f of arr) {
        if (f.length > 3) {
            for (var i = 0; i < f.length - 2; i++) {
                var tri = [f[0], f[i + 1], f[i + 2]];
                triarr.push(tri);
            }
        }
        else triarr.push(f);
    }
    return triarr;
}

function encloses(p, shape) {
    var intersect = false;
    for (var i = 0, j = shape.length - 1; i < shape.length; j = i++) {
        if (((shape[i].y > p.y) != (shape[j].y > p.y)) && (p.x < (shape[j].x - shape[i].x)*(p.y - shape[i].y)/(shape[j].y - shape[i].y) + shape[i].x)) intersect = !intersect;
    }
    return intersect;
}

function coneArray(radius = 5, segments = 5, height = 5) {
    var circleFace = [];
    for (var t = 0; t < 2*Math.PI; t += 2*Math.PI/segments) {
        var x = radius*Math.cos(t);
        var y = radius*Math.sin(t);
        circleFace.push([x, y, 0]);
    }

    var faces = [circleFace];
    var tip = [0, 0, height];
    for (var i = 0; i < circleFace.length - 1; i++) faces.push([circleFace[i], circleFace[i + 1], tip]);

    return faces;
}

function cubeArray(size) {
    return [
        [[-size, -size, -size], [-size, -size, size], [-size, size, size], [-size, size, -size]],
        [[size, -size, -size], [size, -size, size], [size, size, size], [size, size, -size]],
        [[-size, size, -size], [-size, size, size], [size, size, size], [size, size, -size]],
        [[-size, -size, -size], [-size, -size, size], [size, -size, size], [size, -size, -size]],
        [[-size, -size, -size], [size, -size, -size], [size, size, -size], [-size, size, -size]],
        [[-size, -size, size], [-size, size, size], [size, size, size], [size, -size, size]]
    ];
}

function render() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    var mesh = [];
    for (var j in m) {
        if (m[j].length > 3) {
            for (var i = 0; i < m[j].length - 2; i++) {
                var face = [m[j][0], m[j][i + 1], m[j][i + 2]];
                mesh.push({ id: j, face });
            }
        }
        else mesh.push({ id: j, face: m[j] });
    }
    var projected = [];
    for (var i in mesh) projected.push({ id: mesh[i].id, projectedFace: projectFace(mesh[i].face), interpolate: depthInterpolator(projectFaceWithDepth(mesh[i].face)) });
    for (var x = -canvas.width/4; x < canvas.width/4; x += 12) {
        for (var y = -canvas.height/4; y < canvas.height/4; y += 12) {
            var target = projected.filter((c) => encloses(vec2(x, y), c.projectedFace)).sort((a, b) => a.interpolate(vec2(x, y)) - b.interpolate(vec2(x, y)))[0];
            if (target) {
                var id = parseInt(target.id);
                var variate = id + Math.round((Math.random() - 0.5)*2);
                variate = variate < 0 ? 0 : (variate > 5 ? 5 : variate);
                var char = splitmap[variate][Math.round(Math.random()*(splitmap[variate].length - 1))];
                ctx.fillText(charmap[id % charmap.length], x + canvas.width/2, y + canvas.height/2);
            }
        }
    }
    rotate3(m, vec3(0, Math.sin(t/5)/100, Math.sin(t/5)/100));
    t += Math.PI/100;
}

var m = arrayToMesh(cubeArray(15, 15, 15));
rotate3(m, vec3(0, Math.PI/4, Math.PI/4));
ctx.font = "12px Arial";
ctx.fillStyle = "#aaa";
var charmap = "#*^&!@";
var nf = m.length + 1;
var splitmap = Array.from({ length: nf }, (_, i) => charmap.slice(-i*~(charmap.length/nf), -(i + 1)*~(charmap.length/nf)));
var t = 0;
setInterval(render, 10);