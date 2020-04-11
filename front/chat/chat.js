////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables add by server:
// - urlWebsocket
// - urlRoom
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let CODE_INFO = 0;
let CODE_PEER_JOINED = 1;
let CODE_PEER_LEFT = 2;
let CODE_PEER_MESSAGE = 3;
let CODE_PEER_MESSAGE_FILE = 4;
let CODE_PEER_IS_TYPING = 5;

let CODE_FILE_SHARE = 6;
let CODE_FILE_REQUEST_CHUNK = 7;
let CODE_FILE_CHUNK_DATA = 8;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let socket = new WebSocket(urlWebsocket);
let peedId = null;
let peerName = null;
let peersMap = new Map();
let filesIdCounter = 1;
let filesMap = new Map();
let bulbColorsNumber = 18;
let socketSendBuffer = [];
let lastTimeTypingSent = 0;

setupEmoji();

function nextFileId() {
    return filesIdCounter ++;
}

function insertAtCursor(myField, myValue) {
    //IE support
    if (document.selection) {
        myField.focus();
        sel = document.selection.createRange();
        sel.text = myValue;
    }
    //MOZILLA and others
    else if (myField.selectionStart || myField.selectionStart == '0') {
        var startPos = myField.selectionStart;
        var endPos = myField.selectionEnd;
        myField.value = myField.value.substring(0, startPos)
            + myValue
            + myField.value.substring(endPos, myField.value.length);
    } else {
        myField.value += myValue;
    }
}

function humanFileSize(bytes, spin) {
    var thresh = 1024;
    if(Math.abs(bytes) < thresh) {
        return bytes + ' B';
    }
    var units = ['kB','MB','GB','TB','PB','EB','ZB','YB'];
    var u = -1;
    do {
        bytes /= thresh;
        ++u;
    } while(Math.abs(bytes) >= thresh && u < units.length - 1);
    let result = bytes.toFixed(1) + ' ' + units[u];
    if(spin) {
        let progress = ["↑", "↗", "→", "↘", "↓", "↙", "←", "↖"];
        let i1 = spin % progress.length;
        let i2 = Math.trunc(spin / 10) % progress.length;
        let i3 = Math.trunc(spin / 100) % progress.length;
        result = result + " (" + progress[i3] + progress[i2] + progress[i1] + ")";
    }
    return result;
}

function setupEmoji (){

    let div = document.getElementById('emoji');
    let children = div.children;

    for (let index = 0; index < children.length; index ++) {

        let child = children[index];
        child.addEventListener('click', function() {
            let input = document.getElementById('chat_input');
            insertAtCursor(input, child.textContent);
            input.focus();
        });

    }
}

function postChatMessage(message) {

    removeTypingPeerNow(message.peerId);

    let messageElem;

    let messageField = document.getElementById('chat_history');
    let scrollPos = messageField.scrollHeight - messageField.scrollTop;

    let lastChild = messageField.lastChild;
    if(lastChild) {
        let lastPeerId = lastChild.getAttribute('peerId');
        if(lastPeerId && lastPeerId == message.peerId) {
            messageElem = lastChild;
        }
    }
    if(!messageElem) {
        messageElem = document.createElement('div');
        messageElem.className = "message-container";
        messageElem.setAttribute('peerId', message.peerId);

        let peerName = document.createElement('pre');
        let ts = new Date(message.timestamp / 1000);
        peerName.className = "message-author";
        peerName.textContent = message.peerName + " at " + ts.toLocaleTimeString([], {timeStyle: 'short'});
        messageElem.append(peerName);
        messageField.append(messageElem);
    }

    let messageDiv = document.createElement('div');
    messageDiv.className = "message-div";

    let bulb = document.createElement('div');
    bulb.className = "message-bulb";

    let messageText = document.createElement('pre');
    messageText.className = "message-text";
    messageText.textContent = message.message;

    bulb.append(messageText);
    messageDiv.append(bulb);
    messageElem.append(messageDiv);

    if(scrollPos <= messageField.getBoundingClientRect().height) {
        messageField.scrollTop = messageField.scrollHeight;
    }

}

function postSharedFile(message) {

    let messageElem;

    let messageField = document.getElementById('chat_history');
    let scrollPos = messageField.scrollHeight - messageField.scrollTop;
    let lastChild = messageField.lastChild;
    if(lastChild) {
        let lastPeerId = lastChild.getAttribute('peerId');
        if(lastPeerId && lastPeerId == message.peerId) {
            messageElem = lastChild;
        }
    }
    if(!messageElem) {
        messageElem = document.createElement('div');
        messageElem.className = "message-container";
        messageElem.setAttribute('peerId', message.peerId);

        let peerName = document.createElement('pre');
        let ts = new Date(message.timestamp / 1000);
        peerName.className = "message-author";
        peerName.textContent = message.peerName + " at " + ts.toLocaleTimeString([], {timeStyle: 'short'});
        messageElem.append(peerName);
        messageField.append(messageElem);
    }

    let messageDivFiles = document.createElement('div');
    messageDivFiles.className = "message-div-files";

    for(let i = 0; i < message.files.length; i ++) {

        let file = message.files[i];

        let fileInfoSize = document.createElement('p');
        fileInfoSize.className = "file-info-size";
        fileInfoSize.textContent = "Size: " + humanFileSize(file.size);

        let link = document.createElement('a');
        var linkText = document.createTextNode(file.name);
        link.appendChild(linkText);
        link.href = urlRoom + "/file/" + file.serverFileId;
        link.setAttribute('target', '_blank');

        let messageDivOneFile = document.createElement('div');
        messageDivOneFile.className = "message-div-file";

        messageDivOneFile.append(link);
        messageDivOneFile.append(fileInfoSize);

        if (message.peerId == peerId) {
            let fileInfoSent = document.createElement('pre');
            fileInfoSent.className = "file-info-size";
            fileInfoSent.id = "file_served_" + file.serverFileId;
            fileInfoSent.textContent = "Sent: " + humanFileSize(0, 0);
            fileInfoSent.setAttribute("amount-sent", "0");
            fileInfoSent.setAttribute("progress-spin", "0");
            messageDivOneFile.append(fileInfoSent);

        }

        messageDivFiles.append(messageDivOneFile);

    }

    messageElem.append(messageDivFiles);

    if(scrollPos <= messageField.getBoundingClientRect().height) {
        messageField.scrollTop = messageField.scrollHeight;
    }

}

function postSystemMessage(message) {

    removeTypingPeerNow(message.peerId);

    let messageElem;

    let messageField = document.getElementById('chat_history');
    let scrollPos = messageField.scrollHeight - messageField.scrollTop;
    let lastChild = messageField.lastChild;
    if(lastChild) {
        let lastPeerId = lastChild.getAttribute('peerId');
        if(lastPeerId && lastPeerId == 'sys') {
            messageElem = lastChild;
        }
    }
    if(!messageElem) {
        messageElem = document.createElement('div');
        messageElem.className = "message-container";
        messageElem.setAttribute('peerId', 'sys');
        messageField.append(messageElem);
    }

    let messageDiv = document.createElement('div');
    messageDiv.className = "message-div-system";

    let messageText = document.createElement('pre');
    messageText.className = "message-text";
    messageText.textContent = "📢 " + message.message;

    messageDiv.append(messageText);
    messageElem.append(messageDiv);

    if(scrollPos <= messageField.getBoundingClientRect().height) {
        messageField.scrollTop = messageField.scrollHeight;
    }

}

function postPeerIsTyping(message) {

    if(message.peerId == peerId) {
        return;
    }

    let typingPeerElem = document.getElementById('typing_peer_' + message.peerId);
    if(typingPeerElem && typingPeerElem.classList.contains("typing_peer_removed")) {
        typingPeerElem.remove();
        typingPeerElem = null;
    }
    if(!typingPeerElem) {
        let whosTypingPanel = document.getElementById('chat_whos_typing');
        typingPeerElem = document.createElement('pre');
        typingPeerElem.id = 'typing_peer_' + message.peerId;
        typingPeerElem.className = "typing_peer";
        typingPeerElem.textContent = message.peerName + "   ";
        whosTypingPanel.append(typingPeerElem);
    }

    let ts = new Date();
    typingPeerElem.setAttribute("typing_timestamp", ts.getTime());

}

function removeTypingPeerNow(typingPeerId) {
    let typingPeerElem = document.getElementById('typing_peer_' + typingPeerId);
    if(typingPeerElem) {
        typingPeerElem.remove();
    }
}

function removeTypingPeer(peerElem) {
    let transitionCounter = 0;
    peerElem.classList.add("typing_peer_removed");
    peerElem.addEventListener('transitionstart', function() {
        transitionCounter ++;
    });
    peerElem.addEventListener('transitionend', function() {
        transitionCounter --;
        if(transitionCounter == 0) {
            peerElem.remove();
        }
    });
}

let animateWhosTyping = setInterval(function() {

    let whosTypingPanel = document.getElementById('chat_whos_typing');
    let peers = whosTypingPanel.children;
    let now = (new Date()).getTime();

    for(let i = 0; i < peers.length; i++) {
        let peer = peers[i];
        let timestamp = parseInt(peer.getAttribute("typing_timestamp"));
        if(timestamp + 5000 < now) {
            removeTypingPeer(peer);
        } else {
            let text = peer.textContent;
            if(text.endsWith("...")) {
                peer.textContent = text.replace("...", "   ");
            } else if(text.endsWith("   ")) {
                peer.textContent = text.replace("   ", ".  ");
            } else if(text.endsWith(".  ")) {
                peer.textContent = text.replace(".  ", ".. ");
            } else if(text.endsWith(".. ")) {
                peer.textContent = text.replace(".. ", "...");
            }
        }
    }

}, 500);

function createParticipantElement(peer) {
    let peerElem = document.createElement('div');
    peerElem.id = "peer_" + peer.peerId;
    peerElem.setAttribute("peer_id", peer.peerId);
    peerElem.className = "participant";
    if(peer.peerId == peerId) {
        peerElem.style.backgroundColor = "#E0F7FA";
    } else {
        peerElem.classList.add("peer_style_" + (peer.peerId % bulbColorsNumber));
    }
    let span = document.createElement('span');
    span.textContent = peer.peerName;
    peerElem.append(span);
    return peerElem;
}

function removeParticipantElement(peerElem) {
    let transitionCounter = 0;
    peerElem.classList.add("participant_deleted");
    peerElem.addEventListener('transitionstart', function() {
        transitionCounter ++;
    });
    peerElem.addEventListener('transitionend', function() {
        transitionCounter --;
        if(transitionCounter == 0) {
            peerElem.remove();
        }
    });
}

function addParticipant(peer, parent) {
    parent.append(createParticipantElement(peer));
}

function cmpPeers(a, b) {
    if(a.peerName < b.peerName) { return -1; }
    if(a.peerName > b.peerName) { return 1; }
    return 0;
}

function createParticipantsList() {
    let list = document.getElementById('chat_participants');
    let allPeersElem = document.createElement('div');

    let caption = document.createElement('p');
    caption.id = "participant_count";
    caption.textContent = "Participants: " + peersMap.size;
    caption.className = "participant_n";
    allPeersElem.append(caption);

    let peer = new Object();
    peer.peerId = peerId;
    peer.peerName = peerName;
    addParticipant(peer, allPeersElem);

    allPeersElem.append(document.createElement('hr'));

    let otherPeersElem = document.createElement('div');
    otherPeersElem.id = "peers_other";
    allPeersElem.append(otherPeersElem);

    let peers = Array.from(peersMap.values());
    peers.sort(cmpPeers);

    for (let index = 0; index < peers.length; index++) {

        let peer = peers[index];

        if (peer.peerId !== peerId) {
            addParticipant(peer, otherPeersElem);
        }
    }

    list.innerHTML = allPeersElem.innerHTML;
}

function updateParticipants() {

    let list = document.getElementById('chat_participants');
    if(list.innerHTML === "") {
        createParticipantsList();
    } else {

        let countElem = document.getElementById('participant_count');
        countElem.textContent = "Participants: " + peersMap.size;

        let peers = Array.from(peersMap.values());
        peers.sort(cmpPeers);

        let list = document.getElementById('peers_other');
        let children = list.children;
        let childrenLeft = [];
        let childrenNew = [];

        for (let index = 0; index < children.length; index ++) {

            let child = children[index];
            if(!peersMap.get(parseInt(child.getAttribute("peer_id")))) {
                removeParticipantElement(child);
            } else {
                childrenLeft.push(child);
            }

        }

        let peerIndex = 0;

        for (let index = 0; index < childrenLeft.length; index ++) {

            let child = childrenLeft[index];
            let childId = parseInt(child.getAttribute("peer_id"));
            let inserted = true;
            while(inserted) {
                let peer = peers[peerIndex];
                if(peer.peerId !== peerId) {
                    if (peer.peerId !== childId) {
                        let newChild = createParticipantElement(peer);
                        newChild.classList.add("peer_style_new");
                        list.insertBefore(newChild, child);
                        childrenNew.push(newChild);
                    } else {
                        inserted = false;
                    }
                }
                peerIndex++;
            }
        }

        for (let index = peerIndex; index < peers.length; index ++) {
            let peer = peers[index];
            if(peer.peerId !== peerId) {
                let newChild = createParticipantElement(peer);
                newChild.classList.add("peer_style_new");
                list.append(newChild);
                childrenNew.push(newChild);
            }
        }

        for (let index = 0; index < childrenNew.length; index ++) {
            let child = childrenNew[index];
            window.getComputedStyle(child).opacity;
            childrenNew[index].classList.remove("peer_style_new");
        }

    }

}

function sendFileChunks(message) {

    for(let i = 0; i < message.files.length; i++) {

        let chunkInfo = message.files[i];

        let file = filesMap.get(chunkInfo.clientFileId);
        if (file) {

            var posEnd = chunkInfo.chunkPosition + chunkInfo.chunkSize;
            if (posEnd > file.size) {
                posEnd = file.size;
            }

            let chunk = file.slice(chunkInfo.chunkPosition, posEnd);

            var reader = new FileReader();
            reader.readAsBinaryString(chunk);
            reader.onloadend = function () {

                let data = btoa(reader.result);

                let chunkData = {
                    serverFileId: chunkInfo.serverFileId,
                    subscriberId: chunkInfo.subscriberId,
                    data: data
                }

                let message = {
                    peerId: peerId,
                    code: CODE_FILE_CHUNK_DATA,
                    files: [chunkData]
                }

                socketSendNextData(JSON.stringify(message));

                let chunkSize = posEnd - chunkInfo.chunkPosition;
                let sentLabel = document.getElementById("file_served_" + chunkInfo.serverFileId);
                let sent = parseInt(sentLabel.getAttribute("amount-sent")) + chunkSize;
                let spin = parseInt(sentLabel.getAttribute("progress-spin")) + 1;
                sentLabel.setAttribute("amount-sent", sent);
                sentLabel.setAttribute("progress-spin", spin);
                sentLabel.textContent = "Sent: " + humanFileSize(sent, spin);

            }

        }

    }

}

function handleFiles(files) {

    let filesJson = [];

    for(let index = 0; index < files.length; index ++ ) {

        let file = files[index];
        let fileId = nextFileId();

        filesMap.set(fileId, file);

        filesJson.push({
            name: file.name,
            clientFileId: fileId,
            size: file.size
        });

    }

    let message = {code: CODE_FILE_SHARE, files: filesJson};
    socketSendNextData(JSON.stringify(message));

    document.getElementById('file_share_button').value = "";

}

// send message from the form
document.forms.publish.onsubmit = function() {
    let outgoingMessage = this.message.value;

    let text = outgoingMessage.replace(/\s/g,''); // check if text not empty (remove all whitespaces)

    if(text !== "") {
        let message = {
            peerId: peerId,
            peerName: peerName,
            code: CODE_PEER_MESSAGE,
            message: outgoingMessage
        }
        socketSendNextData(JSON.stringify(message));
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

document.getElementById('chat_input').addEventListener("input", function () {

    let now = (new Date()).getTime();

    if(now > lastTimeTypingSent + 1000) {
        let message = {
            code: CODE_PEER_IS_TYPING,
        }
        socketSendNextData(JSON.stringify(message));
        lastTimeTypingSent = now;
    }

});

socket.onclose = function(event) {
    let status = document.getElementById('status_connection');
    status.textContent = "offline";
    status.className = "status_offline";
    peersMap.clear();
    updateParticipants();
};

// message received - show the message in div#messages
socket.onmessage = function(event) {
    onMessage(JSON.parse(event.data));
}

function onMessage(message) {

    switch(message.code) {

        case CODE_INFO:

            peerId = message.peerId;
            peerName = message.peerName;

            for (let index = 0; index < message.peers.length; index++) {
                let peer = message.peers[index];
                peersMap.set(peer.peerId, peer);
            }

            updateParticipants();

            if(message.history && message.history.length > 0) {
                for (let index = 0; index < message.history.length; index++) {
                    onMessage(message.history[index]);
                }
            } else {
                onMessage({
                    code: CODE_PEER_JOINED,
                    peerId: peerId,
                    peerName: peerName,
                    message: peerName + " - joined room"
                });
            }

            break;

        case CODE_PEER_JOINED:
            postSystemMessage(message);
            peer = new Object();
            peer.peerId = message.peerId;
            peer.peerName = message.peerName;
            peersMap.set(peer.peerId, peer);
            updateParticipants();
            break;

        case CODE_PEER_LEFT:
            postSystemMessage(message);
            peersMap.delete(message.peerId);
            updateParticipants();
            break;

        case CODE_PEER_MESSAGE:
            postChatMessage(message);
            break;

        case CODE_PEER_IS_TYPING:
            postPeerIsTyping(message)
            break;

        case CODE_PEER_MESSAGE_FILE:
            postSharedFile(message);
            break;

        case CODE_FILE_REQUEST_CHUNK:
            sendFileChunks(message);
            break;

    }
}

function socketSendNextData(data) {
    socket.send(data);
}

window.addEventListener("beforeunload", function (e) {
    event.preventDefault();
    event.returnValue = "You are about to leave the chat. " +
        "Once you leave you'll lose chat history and all of your files shared will be canceled. " +
        "Are you sure you want to leave the chat?";
});
