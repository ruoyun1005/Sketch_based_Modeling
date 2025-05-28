const canvas = document.getElementById('draw');
const ctx = canvas.getContext('2d');
let drawing = false;
let points = [];
let lines = [];
let startX = 0;
let startY = 0;

canvas.addEventListener('mousedown', (e) => {
    const rect = canvas.getBoundingClientRect();
    startX = e.clientX - rect.left;
    startY = e.clientY - rect.top;
    isDrawing = true;
});

canvas.addEventListener('mouseup', (e) => {
    if (!isDrawing) return;
    const rect = canvas.getBoundingClientRect();
    const endX = e.clientX - rect.left;
    const endY = e.clientY - rect.top;

    ctx.beginPath();
    ctx.moveTo(startX, startY);
    ctx.lineTo(endX, endY);
    ctx.stroke();

    lines.push([[startX, startY], [endX, endY]]);
    isDrawing = false;
});



function downloadPoints() {
    const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(lines));
    const a = document.createElement('a');
    a.href = dataStr;
    a.download = "lines.json";
    a.click();
}

function upLoadToServer(){
    fetch("http://localhost:5000/upload", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(lines)
    })
}
