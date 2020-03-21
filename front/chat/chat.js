let socket = new WebSocket("%%%URL%%%");

// send message from the form
document.forms.publish.onsubmit = function() {
    let outgoingMessage = this.message.value;
    socket.send(outgoingMessage);
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
    let message = event.data;

    let messageElem = document.createElement('div');
    messageElem.className = "message-container";

    let messageDiv = document.createElement('div');
    messageDiv.className = "message-div";

    let peerName = document.createElement('p');
    peerName.className = "message-author";
    peerName.textContent = "Mr.X";

    let messageText = document.createElement('p');
    messageText.className = "message-text";
    messageText.textContent = message;

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

