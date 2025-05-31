function showMessage() {
    alert('Hello from bundled JS!');
}

document.addEventListener('mousemove', animate);
document.addEventListener('mousedown', animate);
document.addEventListener('click', animate);

function animate() {
    document.getElementById('button').style.color =  `rgb(${Math.round(Math.random()*255)}, 0, 0)`
    requestAnimationFrame(animate);
}