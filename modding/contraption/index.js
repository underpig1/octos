// initialize
const { Engine, Render, Runner, Bodies, Composite, Mouse, MouseConstraint, Constraint, Events, Body } = Matter;

const canvas = document.getElementById('canvas');
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;
const vec = (x, y) => Matter.Vector.create(x, y)

const engine = Engine.create();
engine.gravity.y = 1;

const render = Render.create({
    engine: engine,
    canvas: canvas,
    options: {
        width: window.innerWidth,
        height: window.innerHeight,
        wireframes: false,
        background: 'white'
    }
});

const mouse = Mouse.create(document.body);
const mouseConstraint = MouseConstraint.create(engine, {
    mouse: mouse,
    constraint: {
        stiffness: 0.2,
        render: { visible: false }
    }
});

Composite.add(engine.world, mouseConstraint);

Render.run(render);
const runner = Runner.create();
Runner.run(runner, engine);

// menu controls
let paused = false;
const pauseBtn = document.getElementById('pauseBtn');

function freezeBody(body) {
    body.isPaused = true;
    body.isSensor = true;
    Matter.Body.setVelocity(body, { x: 0, y: 0 });
    Matter.Body.setAngularVelocity(body, 0);
}

function togglePause() {
    paused = !paused;
    const el = document.getElementById('pause-btn')
    if (paused) {
        el.className = 'ri-play-large-line'
        const bodies = Matter.Composite.allBodies(engine.world).filter(b => !b.isStatic);
        bodies.forEach(body => {
            freezeBody(body)
        });
        engine.world.gravity.y = 0
    }
    else {
        el.className = 'ri-pause-line'
        const bodies = Matter.Composite.allBodies(engine.world).filter(b => b.isPaused);
        bodies.forEach(body => {
            body.isPaused = false;
            if (!body.isPermanentSensor)
                body.isSensor = false;
        });
        engine.world.gravity.y = settings.gravity;
    }
}

let prevSettings = {};
function updateSettings() {
    if (!paused)
        engine.world.gravity.y = settings.gravity;
    if (prevSettings.weightFactor != settings.weightFactor) {
        for (const body of Composite.allBodies(engine.world)) {
            if (body.isWeight) {
                Matter.Body.setDensity(body, (settings.weightFactor || 1) * 0.1)
            }
        }
    }
    for (const id of Object.keys(settings)) {
        const el = document.getElementById(id);
        if (el)
            el.value = settings[id];
    }
    if (prevSettings.theme != settings.theme)
        setThemeElement(settings.theme)
    prevSettings = { ...settings }
}

function onSelect(el, e) {
    settings[el.id] = parseFloat(e.target.value);
    updateSettings();
}

document.body.addEventListener('click', (e) => {
    if (!document.getElementById('theme-menu').contains(e.target) && !document.getElementById('settings-menu').contains(e.target))
        closeAllMenus()
})

let wasPausedBefore = false;
function openMenu(id, e) {
    const el = document.getElementById(id)
    if (el.classList.contains('active')) {
        el.classList.remove('active')
        return;
    }
    for (const menu of document.getElementById('menus').children) {
        if (menu.id != id)
            menu.classList.remove('active');
        else
            menu.classList.add('active');
    }
    if (id == 'add-menu') {
        wasPausedBefore = paused
        if (!paused)
            togglePause()
    }
    e.stopPropagation()
}

function closeAllMenus() {
    for (const el of document.getElementsByClassName('menu')) {
        closeMenu(el)
    }
}

function closeMenu(el) {
    el.classList.remove('active')
}

// adding entities
let holdingBody = false;
// let wasPausedBeforeHolding = false;
function addBody(body) {
    if (paused) freezeBody(body)
    Composite.add(engine.world, body);
    holdingBody = body;
    // wasPausedBeforeHolding = paused
    // if (!paused)
    //     togglePause()
}

function pickRandomFillStyle() {
    return Array.isArray(fillStyle) ? fillStyle[Math.floor(Math.random() * fillStyle.length)] : fillStyle
}

function addBasicShape(shape = 'rect', fixed = false) {
    const pos = mouse.position
    const body = shape == 'rect' ? Bodies.rectangle(pos.x, pos.y, 150, 75) : Bodies.circle(pos.x, pos.y, 40)
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    if (fixed) {
        body.isStatic = true
        body.isFixed = true
        body.isStatic = true
    }
    addBody(body)
}

function addBomb() {
    const pos = mouse.position
    const body = Bodies.circle(pos.x, pos.y, 20);
    body.render = {
        fillStyle: 'black',
        lineWidth,
        strokeStyle
    };
    body.initialTimer = 3000
    body.timer = 3000;
    body.isBomb = true;
    addBody(body)
}

function addRadialForceSource(dir = 1) {
    const pos = mouse.position
    const body = Bodies.rectangle(pos.x, pos.y, 30, 30);
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    Matter.Body.setDensity(body, 1);
    body.radialForceSourceDir = dir
    addBody(body)
}

function addWeight() {
    const pos = mouse.position
    const body = Bodies.circle(pos.x, pos.y, 20);
    body.render = {
        fillStyle: 'gray',
        lineWidth,
        strokeStyle
    };
    body.isWeight = true;
    Matter.Body.setDensity(body, settings.weightFactor * 0.1);
    addBody(body)
}

function addIce() {
    const pos = mouse.position
    const body = Bodies.rectangle(pos.x, pos.y, 50, 50);
    body.render = {
        fillStyle: iceStyle,
        lineWidth,
        strokeStyle
    };
    body.friction = 0;
    body.frictionAir = 0;
    body.frictionStatic = 0;
    body.isIce = true;
    addBody(body)
}

function addPortal() {
    const pos = mouse.position
    const body = Bodies.polygon(pos.x, pos.y, 6, 25);
    addBody(body)
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    body.isPermanentSensor = true;
    body.isPortal = true;
    body.isStatic = true;
    body.isSensor = true;
}

function addDuplicator() {
    const pos = mouse.position
    const body = Bodies.polygon(pos.x, pos.y, 8, 25);
    Body.setAngle(body, Math.PI / 2)
    addBody(body)
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    body.isPermanentSensor = true;
    body.isDuplicator = true;
    body.isStatic = true;
    body.isSensor = true;
}

function addSpawner() {
    const pos = mouse.position
    const body = Bodies.polygon(pos.x, pos.y, 3, 25);
    Body.setAngle(body, -Math.PI / 2)
    addBody(body)
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    body.isPermanentSensor = true;
    body.isSpawner = true;
    body.lastSpawnTime = 0;
    body.isStatic = true;
    body.isSensor = true;
}

// handle teleport
Matter.Events.on(engine, 'collisionStart', (event) => {
    const shuffle = (array) => {
        const result = array.slice();
        for (let i = result.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [result[i], result[j]] = [result[j], result[i]];
        }
        return result;
    };

    const getChainCenter = (bodies) => {
        let sumX = 0;
        let sumY = 0;
        bodies.forEach(body => {
            sumX += body.position.x;
            sumY += body.position.y;
        });
        const centerX = sumX / bodies.length;
        const centerY = sumY / bodies.length;
        return vec(centerX, centerY);
    }

    const handlePortal = (fromPortal, targetBody) => {
        if (targetBody.justPortaled || targetBody.isDragging)
            return;
        let targetPortal;
        const targetChain = getFastenedChain(targetBody);
        for (const body of shuffle(Composite.allBodies(engine.world))) {
            if (body.isPortal && body != fromPortal) {
                targetPortal = body;
                break;
            }
        }
        if (targetPortal) {
            const chainCenter = getChainCenter(targetChain);
            const offset = Matter.Vector.sub(targetPortal.position, chainCenter);
            for (const b of targetChain) {
                Matter.Body.setPosition(b, Matter.Vector.add(b.position, offset))
                b.justPortaled = true;
                setTimeout(() => {
                    b.justPortaled = false;
                }, 100)
            }
        }
        else {
            for (const b of targetChain) {
                removeBody(targetBody);
            }
        }
    }

    const handleDuplicate = (fromPortal, targetBody) => {
        if (targetBody.justPortaled || targetBody.isDragging)
            return;
        let targetPortal;
        const targetChain = getFastenedChain(targetBody);
        for (const body of shuffle(Composite.allBodies(engine.world))) {
            if (body.isDuplicator && body != fromPortal) {
                targetPortal = body;
                break;
            }
        }
        for (const b of targetChain) {
            b.justPortaled = true;
            setTimeout(() => {
                b.justPortaled = false;
            }, 1000)
        }
        if (targetPortal) {
            const chainCenter = getChainCenter(targetChain);
            const offset = Matter.Vector.sub(targetPortal.position, chainCenter);
            const newChain = cloneChain(targetBody)
            for (const b of newChain) {
                Matter.Body.setPosition(b, Matter.Vector.add(b.position, offset))
                b.justPortaled = true;
                setTimeout(() => {
                    b.justPortaled = false;
                }, 1000)
            }
        }
        else {
            const newChain = cloneChain(targetBody)
            for (const b of newChain) {
                Matter.Body.setPosition(b, Matter.Vector.add(b.position, vec(0, -100)))
                b.justPortaled = true;
                setTimeout(() => {
                    b.justPortaled = false;
                }, 1000)
            }
        }
    }

    if (paused)
        return;

    const pairs = event.pairs;
    for (const pair of pairs) {
        const bodyA = pair.bodyA;
        const bodyB = pair.bodyB;
        if (bodyA.isPortal && !bodyB.isPortal) {
            handlePortal(bodyA, bodyB)
        }
        else if (bodyB.isPortal && !bodyA.isPortal) {
            handlePortal(bodyB, bodyA)
        }
        else if (bodyA.isDuplicator && !bodyB.isDuplicator) {
            handleDuplicate(bodyA, bodyB)
        }
        else if (bodyB.isDuplicator && !bodyA.isDuplicator) {
            handleDuplicate(bodyB, bodyA)
        }
    }
});

function addEntity(entity) {
    switch (entity) {
        case 'rect':
            addBasicShape('rect', false)
            break;
        case 'circle':
            addBasicShape('circle', false)
            break;
        case 'fixed-rect':
            addBasicShape('rect', true)
            break;
        case 'fixed-circle':
            addBasicShape('circle', true)
            break;
        case 'connect':
            holdingFastener = true;
            fasteners.push({
                type: 'connect',
                holding: true
            })
            break;
        case 'hinge':
            holdingFastener = true;
            fasteners.push({
                type: 'hinge',
                holding: true
            })
            break;
        case 'motor':
            holdingFastener = true;
            fasteners.push({
                type: 'motor',
                holding: true
            })
            break;
        case 'r-motor':
            holdingFastener = true;
            fasteners.push({
                type: 'r-motor',
                holding: true
            })
            break;
        case 'attractor':
            addRadialForceSource(-1);
            break;
        case 'repeller':
            addRadialForceSource(1);
            break;
        case 'weight':
            addWeight();
            break;
        case 'explosive':
            addBomb();
            break;
        case 'ice':
            addIce();
            break;
        case 'portal':
            addPortal();
            break;
        case 'duplicator':
            addDuplicator();
            break;
        case 'spawner':
            addSpawner();
            break;
        default:
            break;
    }
}

// examples
function createSevenSegmentRects(w, h, t) {
    const halfh = h / 2;
    const verth = h - t * 3
    const halfw = w / 2;
    return {
        // Horizontal segments
        A: { x: -halfw, y: -halfh, width: w, height: t },
        D: { x: -halfw, y: halfh - t, width: w, height: t },
        G: { x: -halfw, y: -t / 2, width: w, height: t },
        // Vertical segments
        B: { x: halfw - t + t, y: -halfh + t, width: t, height: verth / 2 },
        C: { x: halfw - t + t, y: t / 2, width: t, height: verth / 2 },
        E: { x: -halfw - t, y: t / 2, width: t, height: verth / 2 },
        F: { x: -halfw - t, y: -halfh + t, width: t, height: verth / 2 },
        COLON_TOP: { x: -halfw, y: -halfh / 2, width: t, height: t },
        COLON_BOTTOM: { x: -halfw, y: halfh / 2, width: t, height: t }
    };
}


const DIGIT_SEGMENTS = {
    0: ['A', 'B', 'C', 'D', 'E', 'F'],
    1: ['B', 'C'],
    2: ['A', 'B', 'G', 'E', 'D'],
    3: ['A', 'B', 'C', 'D', 'G'],
    4: ['F', 'G', 'B', 'C'],
    5: ['A', 'F', 'G', 'C', 'D'],
    6: ['A', 'F', 'E', 'D', 'C', 'G'],
    7: ['A', 'B', 'C'],
    8: ['A', 'B', 'C', 'D', 'E', 'F', 'G'],
    9: ['A', 'B', 'C', 'D', 'F', 'G'],
    ':': ['COLON_TOP', 'COLON_BOTTOM']
};

function createNumberRect(pos, n, width, height, thickness) {
    const segments = createSevenSegmentRects(width, height, thickness);
    const active = DIGIT_SEGMENTS[n] || [];
    const rects = active.map(seg => segments[seg])
    const bodies = []
    for (const rect of rects) {
        const body = Bodies.rectangle(rect.x + pos.x + rect.width / 2, rect.y + pos.y + rect.height / 2, rect.width, rect.height);
        body.render = {
            fillStyle: pickRandomFillStyle(),
            lineWidth,
            strokeStyle
        };
        Matter.Body.setDensity(body, 0.01)
        // body.isStatic = true;
        Composite.add(engine.world, body);
        bodies.push(body)
    }
    return bodies
}

function removeAllBodies() {
    const bodies = Composite.allBodies(engine.world)
    for (const body of bodies) {
        removeBody(body)
    }
    if (timeInterval) {
        clearInterval(timeInterval)
        timeInterval = null;
    }
}

function clearScene() {
    removeAllBodies();
    createAndAddGround();
}

function createTimeRects() {
    removeAllBodies();
    const now = new Date();
    const timeString = now.getHours().toString().padStart(2, '0') + ':' +
        now.getMinutes().toString().padStart(2, '0');
    const gap = 50, width = 100, height = 250, thickness = 15;
    let pos = vec(window.innerWidth / 2, window.innerHeight / 2)
    let totalWidth = (timeString.length - 1) * width + (timeString.length - 2) * gap + thickness - width / 2
    pos.x -= totalWidth / 2
    const bodies = []
    for (const n of timeString) {
        const b = createNumberRect(pos, n, width, height, thickness)
        pos.x += (n == ':' ? thickness : width) + gap
        bodies.push(...b)
    }
}

function createTimeRectsDestroyer() {
    // if (Math.random() > 0.5) {
    //     const destroyer = Bodies.circle(window.innerWidth/2, window.innerHeight/2, 5, 5);
    //     destroyer.render = {
    //         fillStyle: pickRandomFillStyle(),
    //         lineWidth,
    //         strokeStyle
    //     };
    //     destroyer.initialTimer = 2000
    //     destroyer.timer = 2000;
    //     destroyer.isBomb = true;
    //     Composite.add(engine.world, destroyer);
    //     return;
    // }
    const o = [-1, 0, 1];
    let i = Math.floor(Math.random() * 8);
    if (i >= 4) i++;
    const dir = vec(o[Math.floor(i / 3)], o[i % 3]);
    const startY = window.innerHeight / 2 + dir.y * (window.innerHeight / 2 + 25)
    const startX = window.innerWidth / 2 + dir.x * (window.innerWidth / 2 + 25)
    const destroyer = Bodies.rectangle(startX, startY, 30, 30);
    destroyer.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    Matter.Body.setDensity(destroyer, 5);
    destroyer.radialForceSourceDir = Math.random() < 0.5 ? -1 : 1
    Matter.Body.setVelocity(destroyer, vec(dir.x * -20, dir.y * -20))
    Composite.add(engine.world, destroyer);
    if (Math.random() > 0.5) createTimeRectsDestroyer()
}

let timeInterval;
function startTime() {
    settings.gravity = 0
    engine.world.gravity.y = 0
    createTimeRects();
    let lastTime = new Date().getMinutes();
    timeInterval = setInterval(() => {
        const currentTime = new Date().getMinutes();
        if (lastTime != currentTime) {
            createTimeRectsDestroyer();
            setTimeout(() => {
                createTimeRects()
            }, 3000)
            lastTime = currentTime
        }
    }, 1000)
}

function addVirtualBody(body, fixed = false) {
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    if (fixed) {
        body.isFixed = true;
        body.isStatic = true;
    }
    if (paused) freezeBody(body)
    Composite.add(engine.world, body);
}

function addVirtualFastener(type, bodyA, bodyB, position) {
    const fastener = { type, holding: false }
    addFastener(fastener, bodyA, bodyB, position);
    fasteners.push(fastener);
}

function addVirtualPortal(pos) {
    const body = Bodies.polygon(pos.x, pos.y, 6, 25);
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    body.isPermanentSensor = true;
    body.isPortal = true;
    body.isStatic = true;
    body.isSensor = true;
    if (paused) freezeBody(body)
    Composite.add(engine.world, body);
}

function addVirtualSpawner(pos) {
    const body = Bodies.polygon(pos.x, pos.y, 3, 25);
    Body.setAngle(body, -Math.PI / 2)
    body.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    body.isPermanentSensor = true;
    body.isSpawner = true;
    body.lastSpawnTime = 0;
    body.isStatic = true;
    body.isSensor = true;
    if (paused) freezeBody(body)
    Composite.add(engine.world, body);
}

function resetSettings() {
    settings.gravity = 1;
    settings.weightFactor = 1;
    settings.attractionPower = 1;
    settings.motorSpeed = 1;
    settings.explosivePower = 1;
    settings.spawnRate = 0.5;
    settings.spawnSpeed = 10;
    updateSettings();
}

function createCarScene() {
    resetSettings();
    removeAllBodies();
    createAndAddGround()
    const center = vec(window.innerWidth / 2, window.innerHeight / 2);
    const carPos = vec(center.x - 200, 0)
    const wheelA = Bodies.circle(carPos.x - 50, carPos.y, 40)
    addVirtualBody(wheelA);
    const wheelB = Bodies.circle(carPos.x + 50, carPos.y, 40)
    Body.setMass(wheelB, 50);
    addVirtualBody(wheelB);
    const chassis = Bodies.rectangle(carPos.x, carPos.y, 115, 25);
    addVirtualBody(chassis);
    addVirtualFastener('hinge', wheelA, chassis, wheelA.position);
    addVirtualFastener('hinge', wheelB, chassis, wheelB.position);

    const obstacleA = Bodies.rectangle(center.x - 200, center.y - 200, 600, 25);
    Body.setAngle(obstacleA, 0.2)
    addVirtualBody(obstacleA, true);
    const obstacleB = Bodies.rectangle(center.x + 200, center.y, 600, 25);
    Body.setAngle(obstacleB, -0.2)
    addVirtualBody(obstacleB, true);
    const obstacleC = Bodies.rectangle(center.x - 200, center.y + 200, 600, 25);
    Body.setAngle(obstacleC, 0.2)
    addVirtualBody(obstacleC, true);

    addVirtualPortal(vec(center.x - 400, center.y - 300));
    addVirtualPortal(vec(center.x + 200, center.y + 200));
}

function createChainScene() {
    resetSettings();
    removeAllBodies();
    const center = vec(window.innerWidth / 2, window.innerHeight / 2);
    const startY = center.y - 200
    const block = Bodies.circle(center.x, startY, 50)
    addVirtualBody(block, true);
    let prevLink = block;
    for (let i = 1; i < 10; i++) {
        const link = Bodies.circle(center.x, startY + i * 60, 50)
        addVirtualBody(link);
        addVirtualFastener('hinge', link, prevLink, vec(link.position.x, link.position.y - 35));
        prevLink = link;
    }
}

function createPistonScene() {
    resetSettings();
    settings.motorSpeed = 4;
    settings.spawnRate = 5;
    removeAllBodies();
    const center = vec(window.innerWidth / 2, window.innerHeight / 2);
    const block = Bodies.circle(center.x - 400, center.y, 50)
    addVirtualBody(block, true);
    const rotator = Bodies.circle(block.position.x, block.position.y, 150)
    addVirtualBody(rotator);
    addVirtualFastener('motor', rotator, block, block.position);
    const arm = Bodies.rectangle(block.position.x + 250, block.position.y - 75, 550, 25)
    Body.setAngle(arm, 0.25)
    addVirtualBody(arm);
    addVirtualFastener('hinge', rotator, arm, vec(block.position.x, block.position.y - 130));
    const piston = Bodies.rectangle(block.position.x + 500, block.position.y, 200, 100)
    addVirtualBody(piston);
    addVirtualFastener('hinge', arm, piston, vec(piston.position.x, piston.position.y));
    const stand = Bodies.rectangle(piston.position.x, piston.position.y + 100, 400, 100)
    addVirtualBody(stand, true);
    const cover = Bodies.rectangle(piston.position.x - 75, piston.position.y - 100, 250, 100)
    addVirtualBody(cover, true);
    excludePair(stand, arm)
    excludePair(block, arm)
    addVirtualSpawner(vec(cover.position.x + 200, cover.position.y - 200))
}

function loadExample(name) {
    switch (name) {
        case 'clock':
            startTime();
            break;
        case 'car':
            createCarScene();
            break;
        case 'chain':
            createChainScene();
            break;
        case 'piston':
            createPistonScene();
            break;
        default:
            break;
    }
}

// theme
function setThemeElement(newTheme) {
    for (const menuItem of document.getElementById('theme-menu-content').children) {
        if (menuItem.id != newTheme + "-theme")
            menuItem.classList.remove('selected')
        else
            menuItem.classList.add('selected')
    }
    setTheme(newTheme);
}

// document.getElementById('fastenBtn').onclick = (e) => {
//     holdingFastener = true;
//     fasteners.push({
//         type: 'motor',
//         holding: true
//     })
//     e.stopPropagation();
// }

let fillStyle, strokeStyle, backgroundColor, controlStyle, stripeStyleA, stripeStyleB, lineWidth, fastenerStyle, bombActiveStyle, iceStyle
function setTheme(t) {
    settings.theme = t
    if (t == 'outline') {
        fillStyle = '#000000aa'
        strokeStyle = 'gray'
        backgroundColor = 'black'
        controlStyle = 'gray'
        stripeStyleA = '#000000aa'
        stripeStyleB = '#000000aa'
        fastenerStyle = 'gray'
        bombActiveStyle = 'gray'
        iceStyle = 'gray'
        lineWidth = 2
    }
    else if (t == 'pencil') {
        fillStyle = ['blue', 'orange', 'green', 'pink', 'yellow', 'gray', 'white', 'red']
        strokeStyle = 'black'
        backgroundColor = '#ff5b5b'
        controlStyle = 'black'
        stripeStyleA = 'yellow'
        stripeStyleB = 'black'
        fastenerStyle = 'black'
        bombActiveStyle = 'red'
        iceStyle = '#a2d2df'
        lineWidth = 5
    }
    else if (t == 'jelly') {
        fillStyle = ['#ffffffaa', '#ff0000aa', '#ffff00aa', '#ff00ffaa', '#0000ffaa', '#00ffffaa', '#00ff00aa']
        strokeStyle = '#000000aa'
        backgroundColor = 'lightblue'
        controlStyle = '#4444ffaa'
        stripeStyleA = '#ffff00aa'
        stripeStyleB = '#000000aa'
        fastenerStyle = '#000000aa'
        bombActiveStyle = 'red'
        iceStyle = '#a2d2df'
        lineWidth = 7
    }
    else if (t == 'dark') {
        fillStyle = '#222'
        strokeStyle = '#333'
        backgroundColor = '#111'
        controlStyle = '#333'
        stripeStyleA = '#222'
        stripeStyleB = '#111'
        fastenerStyle = '#333'
        bombActiveStyle = '#333'
        iceStyle = '#333'
        lineWidth = 4
    }
    else if (t == 'light') {
        fillStyle = '#ddd'
        strokeStyle = '#bbb'
        backgroundColor = '#fff'
        controlStyle = '#bbb'
        stripeStyleA = '#bbb'
        stripeStyleB = '#ddd'
        fastenerStyle = '#aaa'
        bombActiveStyle = '#ccc'
        iceStyle = '#ccc'
        lineWidth = 4
    }
    else if (t == 'simple') {
        fillStyle = ['blue', 'orange', 'green', 'pink', 'yellow', 'white', 'black', 'red', 'cyan', 'purple']
        strokeStyle = 'transparent'
        backgroundColor = 'gray'
        controlStyle = 'darkblue'
        stripeStyleA = 'yellow'
        stripeStyleB = 'black'
        fastenerStyle = 'darkblue'
        bombActiveStyle = 'red'
        iceStyle = '#a2d2df'
        lineWidth = 0
    }
    else if (t == 'blues') {
        fillStyle = ['#0d47a1', '#1976d2', '#42a5f5', '#90caf9', '#bbdefb', '#e3f2fd', '#64b5f6', '#1e88e5', '#1565c0', '#0d47a1']
        strokeStyle = 'transparent'
        backgroundColor = '#0d1b2a'
        controlStyle = '#1565c0'
        stripeStyleA = '#42a5f5'
        stripeStyleB = '#0d47a1'
        fastenerStyle = '#1565c0'
        bombActiveStyle = '#82b1ff'
        iceStyle = '#a2d2df'
        lineWidth = 0
    }
    else if (t == 'reds') {
        fillStyle = ['#b71c1c', '#c62828', '#d32f2f', '#e53935', '#f44336', '#ef5350', '#e57373', '#ef9a9a', '#ffcdd2', '#ff8a80']
        strokeStyle = 'transparent'
        backgroundColor = '#3e0f0f'
        controlStyle = '#b71c1c'
        stripeStyleA = '#f44336'
        stripeStyleB = '#b71c1c'
        fastenerStyle = '#b71c1c'
        bombActiveStyle = '#ff5252'
        iceStyle = '#ffcdd2'
        lineWidth = 0
    }
    else if (t == 'greens') {
        fillStyle = ['#1b5e20', '#2e7d32', '#388e3c', '#43a047', '#4caf50', '#66bb6a', '#81c784', '#a5d6a7', '#c8e6c9', '#e8f5e9']
        strokeStyle = 'transparent'
        backgroundColor = '#102914'
        controlStyle = '#1b5e20'
        stripeStyleA = '#4caf50'
        stripeStyleB = '#1b5e20'
        fastenerStyle = '#1b5e20'
        bombActiveStyle = '#00e676'
        iceStyle = '#c8e6c9'
        lineWidth = 0
    }
    else if (t == 'glass') {
        fillStyle = [
            '#ffffff33',
            '#c8e4ff33',
            '#b4dcff33',
            '#dcf0ff33',
            '#ffffff55',
            '#ffffff1a',
            '#f0faff33',
            '#c8e6ff33',
            '#ffffff26',
            '#d2f0ff33'
        ]
        strokeStyle = 'transparent'
        backgroundColor = '#9ca9b5ff'
        controlStyle = '#4a4a4a'
        stripeStyleA = '#ffffff55'
        stripeStyleB = '#cccccc33'
        fastenerStyle = '#333333'
        bombActiveStyle = '#ff505080'
        iceStyle = '#b4dcff80'
        lineWidth = 0
    }
    else if (t == 'neon') {
        fillStyle = [
            '#00ffffaa', '#00ff99aa', '#ff00ffaa',
            '#ffff00aa', '#ff00ccaa', '#00ffccaa',
            '#33ff00aa', '#ff3300aa', '#ff6600aa', '#33ffffaa'
        ]
        strokeStyle = '#00ffff'
        backgroundColor = '#000000'
        controlStyle = '#00ffff'
        stripeStyleA = '#00ffcc80'
        stripeStyleB = '#ff00ff80'
        fastenerStyle = '#ffffff'
        bombActiveStyle = '#ff00ff'
        iceStyle = '#00ffff55'
        lineWidth = 2
    }
    else if (t == 'lux') {
        fillStyle = [
            '#1a0d2955',
            '#2e126a55',
            '#4b237f66',
            '#5c2e9166',
            '#7a3a9c88',
            '#8d4d9c88',
            '#a05aa999',
            '#bb6eaa99',
            '#d27aabcc',
            '#f9e4aacc'
        ]
        strokeStyle = '#f9e4aaff'
        backgroundColor = '#0a0518ff'
        controlStyle = '#f0d9a3'
        stripeStyleA = '#44246655'
        stripeStyleB = '#77335a55'
        fastenerStyle = '#ccb88c'
        bombActiveStyle = '#e84f4fcc'
        iceStyle = '#a67d9dcc'
        lineWidth = 2
    }
    else if (t == 'space') {
        fillStyle = [
            '#070a1a99',
            '#0a123399',
            '#1c1f4a99',
            '#2b2e6099',
            '#3a3f7bbb',
            '#524d7ebb',
            '#6a6288cc',
            '#7f75a2cc',
            '#9e96b8dd',
            '#bfbadfff'
        ]
        strokeStyle = '#838bb6cc'
        backgroundColor = '#02030fdd'
        controlStyle = '#8899cc'
        stripeStyleA = '#1f224466'
        stripeStyleB = '#36385666'
        fastenerStyle = '#cfcfffcc'
        bombActiveStyle = '#d84343cc'
        iceStyle = '#75a9ffff'
        lineWidth = 2
    }
    else if (t == 'candy') {
        fillStyle = [
            '#ffbad2cc',
            '#ff8fbacc',
            '#ff6faacc',
            '#ff4f99cc',
            '#ff77b2dd',
            '#ff9ccddd',
            '#ffb2d4ee',
            '#ffc8d9ee',
            '#ffdceeee',
            '#ffeeffff'
        ]
        strokeStyle = '#ff5a9bcc'
        backgroundColor = '#ffc0cbff'
        controlStyle = '#ff4a7aff'
        stripeStyleA = '#ffa6c2bb'
        stripeStyleB = '#ff7a9bbb'
        fastenerStyle = '#ff3f81cc'
        bombActiveStyle = '#ff264dcc'
        iceStyle = '#ffd6efff'
        lineWidth = 2
    }
    const bodies = Matter.Composite.allBodies(engine.world);
    for (const body of bodies) {
        if (Array.isArray(fillStyle))
            body.render.fillStyle = fillStyle[Math.floor(Math.random() * fillStyle.length)];
        else
            body.render.fillStyle = fillStyle
        body.render.strokeStyle = strokeStyle;
        if (body.isIce) body.render.fillStyle = iceStyle
    }
    render.options.background = backgroundColor;
    canvas.style.background = backgroundColor;
}

document.addEventListener('DOMContentLoaded', () => {
    setThemeElement('jelly')
    restoreLocal()
    startStorageLoop();
});

// the local state
function serializeBody(body) {
    const cachedAngle = body.angle
    Body.setAngle(body, 0)
    const { parent, parts, id, justPortaled, ...serializedBody } = body
    serializedBody.vertices = body.vertices.map(v => (vec(v.x, v.y)));
    serializedBody.cachedAngle = cachedAngle
    Body.setAngle(body, cachedAngle)
    return serializedBody;
}

function serializeConstraint(constraint) {
    const { bodyA, bodyB, id, ...serializedConstraint } = constraint
    serializedConstraint.bodyAId = bodyA.position
    serializedConstraint.bodyBId = bodyB.position
    return serializedConstraint;
}

function serializeFastener(fastener) {
    const { parent, child, ...serializedFastener } = fastener
    serializedFastener.constraints = [];
    for (const constraint of fastener.constraints) {
        const serializedConstraint = serializeConstraint(constraint)
        serializedFastener.constraints.push(serializedConstraint)
    }
    return serializedFastener
}

function serializeScene() {
    const bodies = Matter.Composite.allBodies(engine.world);
    const serializedBodies = [];
    for (const body of bodies) {
        const serializedBody = serializeBody(body)
        serializedBodies.push(serializedBody);
    }

    const serializedFasteners = [];
    for (const fastener of fasteners) {
        const serializedFastener = serializeFastener(fastener)
        serializedFasteners.push(serializedFastener)
    }
    console.log('SERIALIZING', { bodies: serializedBodies, fasteners: serializedFasteners })
    return { bodies: serializedBodies, fasteners: serializedFasteners }
}

function deserializeAndLoadScene(json) {
    const restoredBodies = json.bodies;
    const newBodies = [];
    console.log('RESTORING FROM ', restoredBodies)
    for (const body of restoredBodies) {
        console.log('RESTORING IS ADDING ', body)
        const newBody = Matter.Body.create(body)
        newBodies.push(newBody)
        Matter.World.add(engine.world, newBody);
        for (const vertex of newBody.vertices) {
            vertex.body = newBody;
        }
        Body.setAngle(newBody, newBody.cachedAngle)
        if (paused) freezeBody(newBody)
    }

    const getBodyFromId = (id) => {
        for (const body of newBodies) {
            if (body.position.x == id.x && body.position.y == id.y)
                return body;
        }
    }

    const restoredFasteners = json.fasteners;
    for (const data of restoredFasteners) {
        console.log('data', data)
        const newConstraints = []
        for (const serializedConstraint of data.constraints) {
            const actualBodyA = getBodyFromId(serializedConstraint.bodyAId)
            const actualBodyB = getBodyFromId(serializedConstraint.bodyBId)
            delete serializedConstraint.bodyAId
            delete serializedConstraint.bodyBId
            const newConstraint = Matter.Constraint.create(serializedConstraint);
            newConstraint.bodyA = actualBodyA
            newConstraint.bodyB = actualBodyB
            if (newConstraint.bodyA && newConstraint.bodyB) {
                newConstraints.push(newConstraint)
                Matter.World.add(engine.world, newConstraint);
            }
        }
        data.constraints = newConstraints;
        data.parent = data.constraints[0].bodyA
        data.child = data.constraints[0].bodyB
        if (data.parent && data.child) {
            for (const constraint of data.constraints) {
                Matter.World.add(engine.world, constraint);
            }
            fasteners.push(data);
            setTimeout(() => {
                console.log('EXCLUDING', data)
                excludePair(data.parent, data.child)
            }, 100)
        }
    }

    // const restoredCategories = json.categories.map(data => [restoreBodyReferences(data[0], bodyMap), data[1]]);
    // console.log(restoredCategories)
    // bodyCategories = new Map(restoredCategories);
    // cleanCategories()
}

function storeLocal() {
    const scene = serializeScene();
    localStorage.setItem("scene", JSON.stringify(scene));
    localStorage.setItem("settings", JSON.stringify(settings));
    // localStorage.setItem("categories", JSON.stringify(serializedCategories));
}

function restoreLocal() {
    const scene = localStorage.getItem("scene")
    if (!scene) {
        createInitialScene();
        return;
    }
    removeAllBodies();
    setTimeout(() => {
        deserializeAndLoadScene(JSON.parse(scene));
        setTheme(settings.theme)
    }, 200)

    const loadedSettings = localStorage.getItem("settings");
    if (loadedSettings) {
        settings = JSON.parse(loadedSettings)
        updateSettings();
    }

    const loadedScenes = localStorage.getItem("scenes");
    if (loadedScenes) {
        savedScenes = JSON.parse(loadedScenes);
        updateSavedScenes();
    }
}

function startStorageLoop() {
    setInterval(() => {
        storeLocal();
    }, 5000)
}

function createAndAddGround() {
    const ground = Bodies.rectangle(
        window.innerWidth / 2,
        window.innerHeight - 30,
        window.innerWidth,
        60,
        { isStatic: true, isFixed: true, render: { fillStyle: "transparent" } }
    );
    Composite.add(engine.world, ground);
}

function createInitialScene() {
    removeAllBodies();
    createAndAddGround();
    resetSettings();

    createCarScene();
    // TODO
    updateSettings();
}

function deleteScene(el, event) {
    console.log('deleting')
    event.stopPropagation();
    const id = el.getAttribute('scene-id')
    if (id && id in savedScenes) {
        delete savedScenes[id];
        localStorage.setItem("scenes", JSON.stringify(savedScenes));
    }
    updateSavedScenes();
}

function loadScene(el, event) {
    if (el.querySelector('.delete-scene-icon').contains(event.target))
        return;
    event.stopPropagation();
    const id = el.getAttribute('scene-id')
    if (id && id in savedScenes) {
        removeAllBodies();
        deserializeAndLoadScene(JSON.parse(JSON.stringify(savedScenes[id])));
        setTheme(settings.theme)
    }
}

function updateSavedScenes() {
    const container = document.getElementById('saved-scene-content');
    container.innerHTML = '';
    for (const id in savedScenes) {
        const el = `<div class="menu-item" onclick="loadScene(this, event)" scene-id="${id}">
    <i class="ri-delete-bin-line delete-scene-icon" onclick="deleteScene(this.parentNode, event)"></i>
    <i class="ri-save-3-line add-item-image"></i>
    <p>Scene ${id}</p>
</div>`
        container.innerHTML += el;
    }
    console.log(savedScenes);
}

function saveScene(event) {
    event.stopPropagation();
    for (let id = 1; id < 13; id++) {
        if (!(id in savedScenes)) {
            savedScenes[id] = serializeScene();
            localStorage.setItem("scenes", JSON.stringify(savedScenes));
            updateSavedScenes();
            return;
        }
    }
}

// object handles
let editingBody = false;
let editingType = 'rotate'
let originalLocalPos = null;
let prevFactor = vec(1, 1);
let savedScenes = {};

function toLocalPos(body, worldPos) {
    const dx = worldPos.x - body.position.x;
    const dy = worldPos.y - body.position.y;
    const cos = Math.cos(-body.angle);
    const sin = Math.sin(-body.angle);
    return {
        x: dx * cos - dy * sin,
        y: dx * sin + dy * cos,
    };
}

function startEditingBody(body, type) {
    editingType = type
    editingBody = body;
    sendToTop(editingBody);
    if (editingBody.isFixed) {
        editingBody.isStatic = false;
        editingBody.isSensor = true;
    }

    if (type == 'scale') {
        originalLocalPos = toLocalPos(editingBody, mouse.position);
        prevFactor = vec(1, 1);
    }
}

function stopEditingBody() {
    if (editingBody.isFixed) {
        editingBody.isStatic = true
        editingBody.isSensor = false;
    }
    editingBody = false
}

function handleBodyEdit() {
    mouseConstraint.constraint.bodyB = null;
    if (editingType == 'rotate') {
        const mouseX = mouse.position.x;
        const mouseY = mouse.position.y;
        const centerX = editingBody.position.x;
        const centerY = editingBody.position.y;
        let angle = Math.atan2(mouseY - centerY, mouseX - centerX) + Math.PI / 2;
        const snap = Math.PI / 4;
        const snapped = Math.round(angle / snap) * snap;
        if (Math.abs(angle - snapped) < Math.PI / 36) angle = snapped;
        Matter.Body.setAngle(editingBody, angle);
    }
    else if (editingType == 'scale') {
        if (originalLocalPos === null) return;
        const localPos = toLocalPos(editingBody, mouse.position);
        const epsilon = 0.0001;
        const newFactorX = localPos.x / (originalLocalPos.x || epsilon);
        const newFactorY = localPos.y / (originalLocalPos.y || epsilon);
        const relativeFactorX = newFactorX / prevFactor.x;
        const relativeFactorY = newFactorY / prevFactor.y;
        const originalAngle = editingBody.angle;
        Matter.Body.setAngle(editingBody, 0);
        Matter.Body.scale(editingBody, relativeFactorX, relativeFactorY);
        Matter.Body.setAngle(editingBody, originalAngle);
        prevFactor.x = newFactorX;
        prevFactor.y = newFactorY;
        for (let i = fasteners.length - 1; i >= 0; i--) {
            const fastener = fasteners[i]
            if (fastener.parent == editingBody || fastener.child == editingBody) {
                const checkOffset = (off) => Matter.Vertices.contains(editingBody.vertices, Matter.Vector.add(getWorldFastenerPosition(fastener), off))
                for (const off of [vec(-10, -10), vec(10, 10)]) {
                    if (checkOffset(off)) return;
                }
                removeFastener(fastener)
                fasteners.splice(i, 1);
            }
        }
    }
}

canvas.addEventListener('mousedown', (e) => {
    const mpos = mouse.position;
    const bodies = [...Matter.Composite.allBodies(engine.world)].reverse();
    for (const b of bodies) {
        if (Matter.Bounds.contains(b.bounds, mpos) && Matter.Vertices.contains(b.vertices, mpos)) {
            return;
        }
        else if (paused) {
            const overHandle = getMouseOverBodyHandle(b);
            if (overHandle) {
                startEditingBody(b, overHandle)
                return;
            }
        }
    }
})
canvas.addEventListener('mouseup', (e) => stopEditingBody());
canvas.addEventListener('mouseleave', (e) => stopEditingBody());
canvas.addEventListener('mousemove', (e) => {
    if (editingBody) {
        handleBodyEdit();
    }
});

// static drag events
let maxZ = 0;
function sendToTop(body) {
    body.zIndex = ++maxZ;
    Composite.remove(engine.world, body);
    Composite.add(engine.world, body);
}

Matter.Events.on(mouseConstraint, 'startdrag', (event) => {
    if (paused && !editingBody) {
        let body;
        const mpos = event.mouse.position;
        const bodies = [...Matter.Composite.allBodies(engine.world)].reverse();
        for (const b of bodies) {
            if (Matter.Bounds.contains(b.bounds, mpos) && Matter.Vertices.contains(b.vertices, mpos)) {
                body = b;
                break;
            }
        }

        if (body) {
            if (!body.isImmovable) {
                body.isDragging = true;
                body.dragOffset = Matter.Vector.sub(mpos, body.position);
            }
            if (!holdingFastener) {
                sendToTop(body);
            }
        }
    }
});

Matter.Events.on(mouseConstraint, 'enddrag', (event) => {
    Matter.Composite.allBodies(engine.world).forEach((body) => {
        if (!body.isImmovable && body.isDragging)
            body.isDragging = false;
    });
});

// collision filters
let bodyCategories = new Map();

function assignCategory(body) {
    if (bodyCategories.has(body)) return;
    const used = new Set([...bodyCategories.values()]);
    for (let bit = 0x0002; bit <= 0x80000000; bit <<= 1) {
        if (!used.has(bit)) {
            bodyCategories.set(body, bit);
            body.collisionFilter.category = bit;
            body.collisionFilter.mask = 0xFFFFFFFF;
            console.log('assigned filter', bodyCategories)
            return;
        }
    }
    throw new Error('Exceeded max unique collision categories (32)');
}

function excludePair(bodyA, bodyB) {
    assignCategory(bodyA);
    assignCategory(bodyB);
    bodyA.collisionFilter.mask &= ~bodyCategories.get(bodyB);
    bodyB.collisionFilter.mask &= ~bodyCategories.get(bodyA);
    // cleanCategories();
}

function includePair(bodyA, bodyB) {
    console.log('trying to include', bodyCategories, bodyA, bodyB)
    if (bodyCategories.has(bodyA) && bodyCategories.has(bodyB)) {
        bodyA.collisionFilter.mask |= bodyCategories.get(bodyB);
        bodyB.collisionFilter.mask |= bodyCategories.get(bodyA);
        bodyCategories.delete(bodyA);
        bodyCategories.delete(bodyB);
        console.log('including')
    }
    // cleanCategories();
}

function cleanCategories() {
    for (const [body, category] of bodyCategories) {
        const found = Composite.allBodies(engine.world).some(target => target.id === body.id);
        if (!found) {
            bodyCategories.delete(body);
        }
    }
}

function addFastener(fastener, bodyA, bodyB, pos = mouse.position) {
    fastener.holding = false;
    fastener.parent = bodyA
    fastener.child = bodyB
    fastener.offset = Matter.Vector.rotate(Matter.Vector.sub(pos, bodyA.position), - bodyA.angle)
    // fastener.childOffset = Matter.Vector.sub(bodyA.position, bodyB.position)

    excludePair(bodyA, bodyB);

    const constraint = Matter.Constraint.create({
        bodyA: bodyA,
        pointA: Matter.Vector.sub(pos, bodyA.position),
        bodyB: bodyB,
        pointB: Matter.Vector.sub(pos, bodyB.position),
        stiffness: fastener.type == 'connect' ? 1 : 0.7,
        length: 0,
        render: {
            visible: false
        },
        offset: vec(0, 0)
    });
    fastener.constraints = [constraint]
    Composite.add(engine.world, constraint);

    if (fastener.type == 'connect') {
        const offsets = [vec(10, 10), vec(-10, -10)]
        for (const offset of offsets) {
            const addedConstraint = Matter.Constraint.create({
                bodyA: bodyA,
                pointA: Matter.Vector.add(Matter.Vector.sub(pos, bodyA.position), offset),
                bodyB: bodyB,
                pointB: Matter.Vector.add(Matter.Vector.sub(pos, bodyB.position), offset),
                stiffness: 1,
                length: 0,
                render: {
                    visible: false
                },
                offset
            });
            fastener.constraints.push(addedConstraint);
            Composite.add(engine.world, addedConstraint);
        }
    }
}

function removeFastener(fastener) {
    for (const constraint of fastener.constraints) {
        Matter.World.remove(engine.world, constraint);
    }
    for (const candidateFastener of fasteners) {
        if (candidateFastener != fastener && (fastener.parent == candidateFastener.parent && fastener.child == candidateFastener.child)
            || (fastener.child == candidateFastener.parent && fastener.parent == candidateFastener.child))
            return;
    }
    includePair(fastener.parent, fastener.child)
    for (const constraint of fastener.constraints) {
        Matter.World.remove(engine.world, constraint);
    }
}

let tooltipTimeout;

// place object
canvas.addEventListener('click', (event) => {
    if (holdingFastener) {
        for (let i = fasteners.length - 1; i >= 0; i--) {
            const fastener = fasteners[i]
            if (fastener.holding) {
                holdingFastener = false;
                const mpos = mouse.position;
                const bodies = [...Matter.Composite.allBodies(engine.world)].reverse().sort((a, b) => (b.isFixed ? 1 : 0) - (a.isFixed ? 1 : 0));
                let bodyA, bodyB
                for (const b of bodies) {
                    if (Matter.Bounds.contains(b.bounds, mpos) && Matter.Vertices.contains(b.vertices, mpos)) {
                        if (bodyA && bodyB)
                            break;
                        else if (bodyA)
                            bodyB = b
                        else
                            bodyA = b
                    }
                }
                if (bodyA && bodyB)
                    addFastener(fastener, bodyA, bodyB);
                else {
                    if (tooltipTimeout)
                        clearTimeout(tooltipTimeout);
                    const tooltip = document.getElementById('tooltip')
                    tooltip.classList.add('active')
                    tooltipTimeout = setTimeout(() => {
                        tooltip.classList.remove('active')
                    }, 3000)
                    fasteners.splice(i, 1);
                }
            }
        }
    }
    else if (holdingBody) {
        holdingBody = false;
        // if (!wasPausedBeforeHolding)
        //     togglePause()
    }
    else if (paused) {
        for (let i = fasteners.length - 1; i >= 0; i--) {
            const fastener = fasteners[i]
            if (isMouseOverFastener(fastener)) {
                removeFastener(fastener)
                fasteners.splice(i, 1);
                return;
            }
        }
    }
});

// render pipeline
function drawTexture(body) {
    ctx.fillStyle = body.render.fillStyle;
    ctx.strokeStyle = body.render.strokeStyle
    ctx.lineWidth = lineWidth
    ctx.beginPath();
    const verts = body.vertices;
    ctx.moveTo(verts[0].x, verts[0].y);
    for (let i = 1; i < verts.length; i++) {
        ctx.lineTo(verts[i].x, verts[i].y);
    }
    ctx.closePath();
    ctx.stroke();
    if (body.isFixed) {
        drawStripedTexture(body)
    }
    else if (body.radialForceSourceDir)
        drawRadialStripedTexture(body, body.radialForceSourceDir)
    else if (body.isPortal || body.isDuplicator)
        drawRadialStripedTexture(body, 1)
    else
        ctx.fill();
}

function getLocalDimensions(body) {
    const cos = Math.cos(-body.angle);
    const sin = Math.sin(-body.angle);
    const localVerts = body.vertices.map(v => {
        const rel = Matter.Vector.sub(v, body.position);
        return {
            x: rel.x * cos - rel.y * sin,
            y: rel.x * sin + rel.y * cos,
        };
    });
    let minX = Infinity,
        maxX = -Infinity,
        minY = Infinity,
        maxY = -Infinity;
    for (const v of localVerts) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
    }
    const width = maxX - minX;
    const height = maxY - minY;
    return [width, height];

}

function contextDrawLocal(body, callback, clip = true) {
    ctx.save();
    if (clip) {
        ctx.beginPath();
        ctx.moveTo(body.vertices[0].x, body.vertices[0].y);
        for (let i = 1; i < body.vertices.length; i++) {
            ctx.lineTo(body.vertices[i].x, body.vertices[i].y);
        }
        ctx.closePath();
        ctx.clip();
    }
    ctx.translate(body.position.x, body.position.y);
    ctx.rotate(body.angle);
    const [width, height] = getLocalDimensions(body)
    callback(width, height);
    ctx.restore();
}

function drawStripedTexture(body) {
    contextDrawLocal(body, (width, height) => {
        const stripeWidth = 20;
        const skew = height / 2;
        const stripeCount = (width + skew) / stripeWidth;
        for (let i = 0; i < stripeCount + 5; i++) {
            ctx.fillStyle = i % 2 === 0 ? stripeStyleA : stripeStyleB;
            ctx.beginPath();
            ctx.moveTo(-width / 2 - skew + i * stripeWidth, -height / 2);
            ctx.lineTo(-width / 2 - skew + i * stripeWidth + stripeWidth, -height / 2);
            ctx.lineTo(-width / 2 - skew + i * stripeWidth + stripeWidth + skew, height / 2);
            ctx.lineTo(-width / 2 - skew + i * stripeWidth + skew, height / 2);
            ctx.closePath();
            ctx.stroke();
            ctx.fill();
        }
    });
}

function drawRadialStripedTexture(body, dir = 1) {
    contextDrawLocal(body, (width, height) => {
        ctx.fill();
        const max_radius = Math.max(width, height)
        const spacing = 20;
        const offset = globalTimer * 10 % (spacing * 2)
        for (let i = 0; i <= Math.ceil(max_radius / spacing); i += 2) {
            let radius = i * spacing + offset * dir;
            // if (inwards) radius = max_radius - radius
            ctx.beginPath();
            ctx.arc(0, 0, radius < 0 ? 0 : radius, 0, Math.PI * 2);
            ctx.arc(0, 0, (radius - spacing * dir) < 0 ? 0 : radius - spacing * dir, 0, Math.PI * 2, true);
            ctx.fillStyle = i % 2 === 0 ? 'black' : 'transparent';
            ctx.fill();
            ctx.beginPath();
            ctx.arc(0, 0, radius < 0 ? 0 : radius, 0, Math.PI * 2);
            ctx.stroke();

            // ctx.beginPath();
            // ctx.arc(0, 0, ringEnd, 0, Math.PI * 2);
            // ctx.stroke();
        }
    });
}

function drawFastener(pos, rot, type) {
    ctx.strokeStyle = fastenerStyle;
    ctx.lineWidth = 3;
    ctx.fillStyle = fastenerStyle;
    ctx.beginPath();
    ctx.arc(pos.x, pos.y, 5, 0, Math.PI * 2);
    ctx.closePath();
    ctx.fill();
    if (type == 'connect') {
        ctx.beginPath();
        ctx.arc(pos.x, pos.y, 8, 0, Math.PI * 2);
        ctx.stroke();
    }
    if (type == 'hinge') {
        for (let i = 0; i < Math.PI * 2; i += Math.PI) {
            ctx.beginPath();
            ctx.arc(pos.x, pos.y, 8, i + rot, i + Math.PI / 2 + rot);
            ctx.stroke();
        }
    }
    else if (type == 'motor' || type == 'r-motor') {
        if (type == 'r-motor') rot *= -1
        for (let i = 0; i < Math.PI * 2; i += Math.PI / 2) {
            ctx.beginPath();
            ctx.arc(pos.x, pos.y, 8, i + rot, i + Math.PI / 4 + rot);
            ctx.stroke();
        }
    }
}

function drawHandles(body) {
    contextDrawLocal(body, (width, height) => {
        ctx.strokeStyle = controlStyle;
        ctx.fillStyle = controlStyle
        ctx.lineWidth = 2;
        // borders
        ctx.beginPath();
        ctx.rect(-width / 2, -height / 2, width, height)
        ctx.stroke();
        // rotate handle
        ctx.moveTo(0, -height / 2)
        ctx.lineTo(0, -height / 2 - 15)
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(0, -height / 2 - 15, 5, 0, Math.PI * 2)
        ctx.fill();
        ctx.beginPath();
        ctx.arc(0, -height / 2 - 15, 8, 0, Math.PI * 2)
        ctx.stroke();
        // resize handle
        ctx.fillRect(width / 2, height / 2, 15, 15)
    }, false);
}

function getMouseOverBodyHandle(body) {
    const [width, height] = getLocalDimensions(body);
    const overLocalToWorld = (localPos, radius) => {
        const localX = 0;
        const localY = -height / 2 - 15;
        const cos = Math.cos(body.angle);
        const sin = Math.sin(body.angle);
        const rotateHandlePos = {
            x: body.position.x + localPos.x * cos - localPos.y * sin,
            y: body.position.y + localPos.x * sin + localPos.y * cos,
        };
        const dx = mouse.position.x - rotateHandlePos.x;
        const dy = mouse.position.y - rotateHandlePos.y;
        return dx * dx + dy * dy <= radius * radius
    }
    if (overLocalToWorld(vec(0, -height / 2 - 15), 16)) {
        return 'rotate'
    }
    else if (overLocalToWorld(vec(width / 2 + 4, height / 2 + 4), 15)) {
        return 'scale'
    }
}

function getWorldFastenerPosition(fastener) {
    const px = fastener.parent.position.x;
    const py = fastener.parent.position.y;
    const theta = fastener.parent.angle;
    const ox = fastener.offset.x;
    const oy = fastener.offset.y;
    const cos = Math.cos(theta);
    const sin = Math.sin(theta);
    const worldX = px + ox * cos - oy * sin;
    const worldY = py + ox * sin + oy * cos;
    return vec(worldX, worldY)
}

function isMouseOverFastener(fastener) {
    const worldPos = getWorldFastenerPosition(fastener);
    const dx = mouse.position.x - worldPos.x;
    const dy = mouse.position.y - worldPos.y;
    return dx * dx + dy * dy <= 12 ** 2;
}

function cloneBody(body) {
    const cachedAngle = body.angle
    Body.setAngle(body, 0)
    const { id, parent, parts, axis, bounds, vertices, ...rest } = body
    for (const vertex of vertices) {
        delete vertex.body
    }
    rest.vertices = vertices
    let newBody = JSON.parse(JSON.stringify(rest));
    Body.setAngle(body, cachedAngle)
    newBody = Matter.Body.create(newBody);
    Matter.Composite.add(engine.world, newBody)
    Body.setAngle(newBody, cachedAngle)
    return newBody
}

function cloneConstraint(constraint, newBodyA, newBodyB) {
    const { id, bodyA, bodyB, ...rest } = constraint
    let newConstraint = JSON.parse(JSON.stringify(rest))
    newConstraint = Matter.Constraint.create(newConstraint);
    newConstraint.bodyA = newBodyA
    newConstraint.bodyB = newBodyB
    Matter.Composite.add(engine.world, newConstraint)
    return newConstraint
}

function cloneFastener(fastener, newBodyA, newBodyB) {
    const newFastener = Object.assign({}, fastener);
    newFastener.constraints = [];
    newFastener.parent = newBodyA
    newFastener.child = newBodyB
    excludePair(newBodyA, newBodyB)
    for (const constraint of fastener.constraints) {
        newFastener.constraints.push(cloneConstraint(constraint, newBodyA, newBodyB))
    }
    fasteners.push(newFastener)
}

// while (true) {
//     localStorage.setItem('bodies', '')
//     clearScene();
// }
function cloneChain(body, visited = new Map(), clonedFasteners = new Set()) {
    if (visited.has(body)) return [];
    const clonedBody = cloneBody(body);
    visited.set(body, clonedBody);
    let clonedChain = [clonedBody];
    for (const fastener of fasteners) {
        if (fastener.parent === body) {
            const childClones = cloneChain(fastener.child, visited, clonedFasteners);
            clonedChain = clonedChain.concat(childClones);
        } else if (fastener.child === body) {
            const parentClones = cloneChain(fastener.parent, visited, clonedFasteners);
            clonedChain = clonedChain.concat(parentClones);
        }
    }
    for (const fastener of fasteners) {
        if (
            visited.has(fastener.parent) &&
            visited.has(fastener.child) &&
            !clonedFasteners.has(fastener)
        ) {
            cloneFastener(
                fastener,
                visited.get(fastener.parent),
                visited.get(fastener.child)
            );
            clonedFasteners.add(fastener);
        }
    }

    return clonedChain;
}

function getFastenedChain(body, visited = new Set()) {
    if (visited.has(body)) return [];
    visited.add(body);
    let chain = [body];
    for (const fastener of fasteners) {
        if (fastener.parent === body) {
            chain = chain.concat(getFastenedChain(fastener.child, visited));
        } else if (fastener.child === body) {
            chain = chain.concat(getFastenedChain(fastener.parent, visited));
        }
    }
    return chain;
}

function removeBody(body) {
    for (let i = fasteners.length - 1; i >= 0; i--) {
        const fastener = fasteners[i]
        if (fastener.parent == body || fastener.child == body) {
            for (const constraint of fastener.constraints) {
                Matter.World.remove(engine.world, constraint);
            }
            fasteners.splice(i, 1);
        }
    }
    Matter.World.remove(engine.world, body);
}

let doRemove = true;
function removeFastenedChainIfOutOfBounds(body) {
    if (!doRemove)
        return;
    const isOutOfBounds = (body) => {
        const bounds = body.bounds;
        const margin = 100
        return (
            bounds.max.x < -margin ||
            bounds.min.x > canvas.width + margin ||
            bounds.max.y < -margin ||
            bounds.min.y > canvas.height + margin
        );
    }
    if (isOutOfBounds(body)) {
        const chain = getFastenedChain(body);
        for (const b of chain) {
            if (!isOutOfBounds(b)) return;
        }
        for (const b of chain) {
            console.log('removing', b)
            removeBody(body);
        }
    }
}

const ctx = render.context;
let globalTimer = 0;
let holdingFastener = false;
const fasteners = []

Events.on(render, 'beforeRender', () => {
    engine.world.bodies = engine.world.bodies.sort((a, b) => (a.zIndex || 0) - (b.zIndex || 0));
});

function applyMotor(bodyA, bodyB, targetRelAngularVel) {
    if (bodyA.id > bodyB.id) [bodyA, bodyB] = [bodyB, bodyA];
    const currentRelVel = bodyB.angularVelocity - bodyA.angularVelocity;
    const error = targetRelAngularVel - currentRelVel;
    const invI1 = bodyA.inverseInertia || 0;
    const invI2 = bodyB.inverseInertia || 0;
    const totalInvI = invI1 + invI2; if (bodyA.isStatic && !bodyB.isStatic) {
        Matter.Body.setAngularVelocity(bodyB, bodyB.angularVelocity + error);
    } else if (!bodyA.isStatic && bodyB.isStatic) {
        Matter.Body.setAngularVelocity(bodyA, bodyA.angularVelocity - error);
    } else if (totalInvI > 0) {
        const correctionA = (error * invI1) / totalInvI;
        const correctionB = (error * invI2) / totalInvI;
        Matter.Body.setAngularVelocity(bodyA, bodyA.angularVelocity - correctionA);
        Matter.Body.setAngularVelocity(bodyB, bodyB.angularVelocity + correctionB);
    }
}

let settings = {}
resetSettings()

function applyRadialForce(position, forceMagnitude, applyToSelf = false) {
    const radius = 1000;
    const bodies = Matter.Composite.allBodies(engine.world);
    bodies.forEach(body => {
        // if (body.radialForceSourceDir)
        //     return;
        const dx = body.position.x - position.x + (applyToSelf ? Math.random() * 2 : 0);
        const dy = body.position.y - position.y + (applyToSelf ? Math.random() * 2 : 0);
        const distSq = dx * dx + dy * dy;
        const radiusSq = radius * radius;
        if (distSq <= radiusSq) {
            const dist = Math.sqrt(distSq) || 0.0001;
            const force = forceMagnitude * (1 - dist / radius);
            const fx = (dx / dist) * force;
            const fy = (dy / dist) * force;
            Matter.Body.applyForce(body, body.position, vec(fx, fy));
        }
    });
}

function spawnBody(parent) {
    const spawn = Bodies.circle(parent.position.x, parent.position.y, 15)
    spawn.render = {
        fillStyle: pickRandomFillStyle(),
        lineWidth,
        strokeStyle
    };
    const speed = settings.spawnSpeed;
    const angle = Math.PI + parent.angle;
    const vel = vec(speed * Math.cos(angle), speed * Math.sin(angle));
    Matter.Body.setVelocity(spawn, vel)
    if (paused) freezeBody(spawn)
    Composite.add(engine.world, spawn);
}

Matter.Render.world = (render) => {
    const ctx = render.context;
    const bodies = Composite.allBodies(render.engine.world);
    ctx.clearRect(0, 0, render.options.width, render.options.height);
    for (const body of bodies) {
        // drawing
        drawTexture(body)
        if (paused && !body.isImmovable) {
            drawHandles(body);
        }

        // dragging
        if (paused && !body.isImmovable && body.isDragging) {
            Body.setPosition(body, Matter.Vector.sub(mouse.position, body.dragOffset));
        }

        // pause
        if (paused) {
            Matter.Body.setVelocity(body, { x: 0, y: 0 });
            Matter.Body.setAngularVelocity(body, 0);
        }
        else {
            if (body.isBomb) {
                body.timer -= engine.timing.lastDelta
                if (body.timer < 0) {
                    applyRadialForce(body.position, 0.2 * settings.explosivePower, true)
                }
                else if (Math.sin(2 * Math.PI * 5 * body.timer / body.initialTimer) > 0)
                    body.render.fillStyle = bombActiveStyle
                else
                    body.render.fillStyle = 'black'
            }
            else if (body.radialForceSourceDir) {
                applyRadialForce(body.position, body.radialForceSourceDir * 0.01 * settings.attractionPower, false);
            }
            else if (body.isSpawner) {
                const interval = 1 / settings.spawnRate;
                const intervalInt = Math.floor(globalTimer / interval);
                if (intervalInt !== body.lastSpawnTime) {
                    spawnBody(body);
                    body.lastSpawnTime = intervalInt;
                }
            }
        }

        // remove if out of scene
        removeFastenedChainIfOutOfBounds(body)
    }

    if (holdingBody) {
        Body.setPosition(holdingBody, mouse.position)
    }

    // draw fasteners
    for (const fastener of fasteners) {
        if (fastener.holding) {
            const mpos = mouse.position;
            drawFastener(mpos, globalTimer, fastener.type);
        }
        else {
            ctx.save();
            ctx.translate(fastener.parent.position.x, fastener.parent.position.y);
            ctx.rotate(fastener.parent.angle);
            drawFastener(fastener.offset, globalTimer, fastener.type);
            ctx.restore();
            if ((fastener.type == 'motor' || fastener.type == 'r-motor') && !paused) {
                applyMotor(fastener.parent, fastener.child, Math.PI / 100 * settings.motorSpeed * (fastener.type == 'r-motor' ? -1 : 1))
                // Body.setAngularVelocity(fastener.parent, -Math.PI / 100 * motorSpeed);
                // Body.setAngularVelocity(fastener.child, Math.PI / 100 * motorSpeed);
            }
        }
    }
    if (!paused)
        globalTimer += Math.PI / 1000 * engine.timing.lastDelta

    // draw tooltips
    if (paused) {

    }
};