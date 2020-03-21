let socket = new WebSocket("%%%URL%%%");
let peedId = null;
let peerName = null;
let peersMap = new Map();
let bulbColors = [  "#FFEBEE", "#FCE4EC", "#F3E5F5",
                    "#EDE7F6", "#E8EAF6", "#E3F2FD",
                    "#E1F5FE",            "#E0F2F1",
                    "#E8F5E9", "#F1F8E9", "#F9FBE7",
                    "#FFFDE7", "#FFF8E1", "#FFF3E0",
                    "#FBE9E7", "#EFEBE9", "#FAFAFA",
                    "#ECEFF1"];

function postChatMessage(message) {

    let messageElem = document.createElement('div');

    if (message.peerId == peerId) {
        messageElem.className = "message-container-me";
    } else {
        messageElem.className = "message-container";
    }

    let messageDiv = document.createElement('div');
    messageDiv.className = "message-div";

    if (message.peerId == peerId) {
        messageDiv.style.backgroundColor = "#E0F7FA";
    } else {
        messageDiv.style.backgroundColor = bulbColors[message.peerId % bulbColors.length];
    }

    let peerName = document.createElement('p');
    let ts = new Date(message.timestamp / 1000);
    peerName.className = "message-author";
    peerName.textContent = message.peerName + " at " + ts.toLocaleString();

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

function postSystemMessage(message) {

    let messageElem = document.createElement('div');
    messageElem.className = "message-container";

    let messageDiv = document.createElement('div');
    messageDiv.className = "message-div-system";
    messageDiv.style.backgroundColor = "#E0F7FA";

    let messageText = document.createElement('pre');
    messageText.className = "message-text";
    messageText.textContent = "📢" + message.message;

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

function updateParticipants() {

    let list = document.getElementById('chat_participants');
    let allPeersElem = document.createElement('div');

    let peerElem = document.createElement('div');
    peerElem.id = "peer_" + peerId;
    peerElem.className = "participant";
    peerElem.textContent = peerName;
    peerElem.style.backgroundColor = "#E0F7FA";
    allPeersElem.append(peerElem);

    allPeersElem.append(document.createElement('hr'));

    let keys = Array.from(peersMap.keys());
    keys.sort();

    for (index = 0; index < keys.length; index++) {

        let id = keys[index];

        if (id !== peerId) {
            let peer = peersMap.get(id);

            let peerElem = document.createElement('div');
            peerElem.id = "peer_" + id;
            peerElem.className = "participant";
            peerElem.textContent = peer.peerName;

            peerElem.style.backgroundColor = bulbColors[peer.peerId % bulbColors.length];

            allPeersElem.append(peerElem);

        }

    }

    list.innerHTML = allPeersElem.innerHTML;

}

// send message from the form
document.forms.publish.onsubmit = function() {
    let outgoingMessage = this.message.value;

    let text = outgoingMessage.replace(/\s/g,''); // check if text not empty (remove all whitespaces)

    if(text !== "") {
        let message = {
            peerId: peerId,
            peerName: peerName,
            code: 3,
            message: outgoingMessage
        }
        socket.send(JSON.stringify(message));
        this.message.value = "";
    }

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

        case 0: // initial info
            peerId = message.peerId;
            peerName = message.peerName;

            for (index = 0; index < message.peers.length; index++) {
                let peer = message.peers[index];
                peersMap.set(peer.peerId, peer);
            }

            updateParticipants();

            break;

        case 1: // joined
            postSystemMessage(message);
            peer = new Object();
            peer.peerId = message.peerId;
            peer.peerName = message.peerName;
            peersMap.set(peer.peerId, peer);
            updateParticipants();
            break;
        case 2: // left
            postSystemMessage(message);
            peersMap.delete(message.peerId);
            updateParticipants();
            break;
        case 3: // message
            postChatMessage(message);
            break;

    }

}

