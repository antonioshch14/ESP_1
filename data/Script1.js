// JavaScript source code
var Socket;
var var1, var2, var3;
var IDforli = 0; //sets IDs for appended li
var maxAppLi = 4;
var newLiElement = [];
var messageLi = document.getElementById("messages");
function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function (event) { processReceivedCommand(event); };
}
document.getElementById('fan').addEventListener('click', startFan);
document.getElementById('airVick').addEventListener('click', pushAirVick);
document.getElementById('fanTimeSubmit').addEventListener('click', fanTimeSubmit);
document.getElementById('light').addEventListener('click', light);

function getValueFromStr(string, field) {
    var beginning = string.indexOf(field) + field.length;
    var ending = string.indexOf(',', beginning);
    return string.substring(beginning, ending);
}
function addNewLi(message) {
    var newLi = document.createElement("li");
    newLi.innerHTML = message;
    newLi.setAttribute('id', "appendedLi" + IDforli);
    messageLi.appendChild(newLi);
    if (IDforli > maxAppLi) {
        newLiElement[0].parentNode.removeChild(newLiElement[0]);
        newLiElement.shift();
        newLiElement.push(document.getElementById("appendedLi" + IDforli));
        newLiElement.forEach(function (item, index) {
            item.setAttribute('id', "appendedLi" + index)
        });
    } else {
        newLiElement[IDforli] = document.getElementById("appendedLi" + IDforli);
        IDforli++;
    }
}
function processReceivedCommand(evt) {
    var recievedData = evt.data;
    document.getElementById('rd').innerHTML = recievedData;
    addNewLi(recievedData);
    
    if (recievedData === 'L_ON') {
        document.getElementById('light').innerHTML = 'Switch OFF';
        document.getElementById('lightState').innerHTML = 'Light is ON';
    }
}



function startFan() {
    var btn = document.getElementById('fan')
    var btnText = btn.innerText;
    if (btnText === 'Switch ON') {
        sendText('fanStart:60000;');
        document.getElementById('fan').innerHTML = 'Switch OFF'; 
        document.getElementById('fanState').innerHTML = ' fan is working';
    } else {
        sendText('fanStop:;');
        document.getElementById('fan').innerHTML = 'Switch ON';
        document.getElementById('fanState').innerHTML = ' fan is not working';
    }

}
function light() {
    var btn = document.getElementById('light')
    var btnText = btn.innerText;
    if (btnText === 'Switch ON') {
        sendText('lightOn:;');
        document.getElementById('light').innerHTML = 'Switch OFF';
        document.getElementById('lightState').innerHTML = ' light is ON';
    } else {
        sendText('lightOff:;');
        document.getElementById('light').innerHTML = 'Switch ON';
        document.getElementById('lightState').innerHTML = ' light is unllit';
    }

}
function pushAirVick() {
    sendText('airvickPush:;');
}
function fanTimeSubmit() {
    var time = document.getElementById('fanTime').value;
    sendText('fanStart:' + time + ';');
    document.getElementById('fanTime').value = '';
}
function sendText(data) {
    Socket.send(data);
}
window.onload = function (e) {
    init();
}
