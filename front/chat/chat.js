let socket = new WebSocket("wss://localhost:8443/ws/chat/my-room/?nickname=mr.x");

// send message from the form
document.forms.publish.onsubmit = function() {
    let outgoingMessage = this.message.value;
    socket.send(outgoingMessage);
    return false;
};

// message received - show the message in div#messages
socket.onmessage = function(event) {
    let message = event.data;

    let messageElem = document.createElement('div');
    messageElem.textContent = message;
    document.getElementById('messages').prepend(messageElem);
}

