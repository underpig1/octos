// function dispatchEventAtPoint(point, eventType) {
//     const element = document.elementFromPoint(point.x, point.y);

//     if (element && element.tagName === "IFRAME") {
//         const rect = element.getBoundingClientRect();
//         const innerPoint = {
//             x: point.x - rect.left,
//             y: point.y - rect.top
//         };
//         const iframeDoc = element.contentDocument || element.contentWindow.document;
//         const target = iframeDoc.elementFromPoint(innerPoint.x, innerPoint.y);
//         if (target) dispatchEvent(target, eventType);
//     }
//     else if (element) dispatchEvent(element, eventType);
// }

// function handleMouseEnter(point) {
//     const elements = document.elementsFromPoint(point.x, point.y);
//     for (const element of elements) {
//         if (!entered.includes(element)) {
//             entered.push(element);
//             dispatchEvent(element, "mouseenter");
//             dispatchEvent(element, "mouseover");
//         }
//     }
//     for (var i = 0; i < entered.length; i++) {
//         if (!elements.includes(entered[i])) {
//             dispatchEvent(entered[i], "mouseleave");
//             dispatchEvent(entered[i], "mouseout");
//             entered.splice(i, 1);
//         }
//     }
// }

// function dispatchEvent(element, type) {
//     console.log(type);
//     const event = new MouseEvent(type, {
//         bubbles: true,
//         button: 0
//     });
//     element.dispatchEvent(event);

// }

// function handleMouseData(mouseData) {
//     const { x, y, pressed } = mouseData;

//     if (lastMouseState.x != x && lastMouseState.y != y) dispatchEventAtPoint({ x, y }, "mousemove");

//     if (pressed) {
//         if (!lastMouseState.pressed) dispatchEventAtPoint({ x, y }, "mousedown");
//     }
//     else if (lastMouseState.pressed) {
//         dispatchEventAtPoint({ x, y }, "mouseup");
//         if (lastMouseState.x == x && lastMouseState.y == y) dispatchEventAtPoint({ x, y }, "click");
//     }

//     lastMouseState = { x, y, pressed };
// }

// var lastMouseState = { x: 0, y: 0, pressed: false };
// var entered = [];

// async function handle() {
//     const mouse = await window.mouse();
//     if (mouse.active) {
//         handleMouseData(mouse);
//         handleMouseEnter(mouse);
//     }
// }

// setInterval(handle, 10);

document.addEventListener("keydown", (e) => {
    console.log(e.key);
})

document.addEventListener("keyup", (e) => {
    if (e.key == " ") e.preventDefault();
})

// setInterval(() => {
//     var position = window.mouse.position();
//     var pressed = window.mouse.pressed() && window.mouse.active();
//     //handleMouseData({ x: data.position.x, y: data.position.y, pressed: data.pressed });
// }, 10);