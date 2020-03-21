let socket = new WebSocket("%%%URL%%%");
let peedId = null;
let peerName = null;
let bulbColors = [  "#FFEBEE", "#FCE4EC", "#F3E5F5",
                    "#EDE7F6", "#E8EAF6", "#E3F2FD",
                    "#E1F5FE",            "#E0F2F1",
                    "#E8F5E9", "#F1F8E9", "#F9FBE7",
                    "#FFFDE7", "#FFF8E1", "#FFF3E0",
                    "#FBE9E7", "#EFEBE9", "#FAFAFA",
                    "#ECEFF1"];

function onChatMessage(message) {

    let messageElem = document.createElement('div');
    if(message.peerId == peerId) {
        messageElem.className = "message-container-me";
    } else {
        messageElem.className = "message-container";
    }

    let messageDiv = document.createElement('div');
    messageDiv.className = "message-div";
    if(message.peerId == peerId) {
        messageDiv.style.backgroundColor = "#E0F7FA";
    } else {
        messageDiv.style.backgroundColor = bulbColors[message.peerId % bulbColors.length];
    }

    let peerName = document.createElement('p');
    peerName.className = "message-author";
    peerName.textContent = message.peerName;

    let messageText = document.createElement('pre');
    messageText.className = "message-text";
    messageText.textContent = message.message;

    messageDiv.append(peerName);
    messageDiv.append(messageText);
    messageElem.append(messageDiv);

    let messageField = document.getElementById('chat_history');
    let scrollPos = messageField.scrollHeight - messageField.scrollTop;

    if(scrollPos <= messageField.getBoundingClientRect().height) {
        messageField.append(messageElem);
        messageField.scrollTop = messageField.scrollHeight;
    } else {
        messageField.append(messageElem);
    }

}

// send message from the form
document.forms.publish.onsubmit = function() {
    let outgoingMessage = this.message.value;
    let message = {
        peerId: peerId,
        peerName: peerName,
        code: 3,
        message: outgoingMessage
    }
    socket.send(JSON.stringify(message));
    this.message.value = "";
    return false;
};

document.getElementById('chat_input').addEventListener("keypress", function (e) {
    if(e.which == 13 && !e.shiftKey) {
        document.forms.publish.onsubmit();
        e.preventDefault();
    }
});

// message received - show the message in div#messages
socket.onmessage = function(event) {

    message = JSON.parse(event.data);

    switch(message.code) {

        case 0:
            peerId = message.peerId;
            peerName = message.peerName;
            break;

        case 1:
        case 2:
        case 3:
            onChatMessage(message);
            break;

    }

}

