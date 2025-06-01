function showMessage() {
    alert('Hello from bundled JS!');
}

document.addEventListener('mousemove', startAnimate);
document.addEventListener('mousedown', startAnimate);
document.addEventListener('click', startAnimate);

var animateStarted = false;
function startAnimate() {
    if (!animateStarted) setInterval(animate, 100);
    animateStarted = true;
}

function animate() {
    document.getElementById('button').style.color =  `rgb(${Math.round(Math.random()*255)}, 0, 0)`
    requestAnimationFrame(animate);
}