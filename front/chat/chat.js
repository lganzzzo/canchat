////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables add by server:
// - urlWebsocket
// - urlRoom
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let socket = new WebSocket(urlWebsocket);
let peedId = null;
let peerName = null;
let peersMap = new Map();
let filesIdCounter = 1;
let filesMap = new Map();
let bulbColorsNumber = 18;
let socketSendBuffer = [];

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

function humanFileSize(bytes) {
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
    return bytes.toFixed(1) + ' ' + units[u];
}

function setupEmoji (){

    let div = document.getElementById('emoji');
    let children = div.children;

    for (index = 0; index < children.length; index ++) {

        let child = children[index];
        child.addEventListener('click', function() {
            let input = document.getElementById('chat_input');
            insertAtCursor(input, child.textContent);
            input.focus();
        });

    }
}

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
        messageDiv.classList.add("peer_style_" + (message.peerId % bulbColorsNumber));
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

function postSharedFile(message) {

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
        messageDiv.classList.add("peer_style_" + (message.peerId % bulbColorsNumber));
    }

    let peerName = document.createElement('p');
    let ts = new Date(message.timestamp / 1000);
    peerName.className = "message-author";
    peerName.textContent = message.peerName + " at " + ts.toLocaleString();

    let fileInfoSize = document.createElement('p');
    fileInfoSize.className = "file-info-size";
    fileInfoSize.textContent = "Size: " + humanFileSize(message.file.size);

    let link = document.createElement('a');
    var linkText = document.createTextNode(message.file.name);
    link.appendChild(linkText);
    link.href = urlRoom + "/file/" + message.file.serverFileId;
    link.setAttribute('target', '_blank');

    messageDiv.append(peerName);
    messageDiv.append(link);
    messageDiv.append(fileInfoSize);

    if (message.peerId == peerId) {
        let fileInfoSent = document.createElement('p');
        fileInfoSent.className = "file-info-served";
        fileInfoSent.id = "file_served_" + message.file.serverFileId;
        fileInfoSent.textContent = "Sent: " + humanFileSize(0);
        fileInfoSent.setAttribute("amount-sent", "0");
        messageDiv.append(fileInfoSent);
    }

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

    for (index = 0; index < peers.length; index++) {

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

        for (index = 0; index < children.length; index ++) {

            let child = children[index];
            if(!peersMap.get(parseInt(child.getAttribute("peer_id")))) {
                removeParticipantElement(child);
            } else {
                childrenLeft.push(child);
            }

        }

        let peerIndex = 0;

        for (index = 0; index < childrenLeft.length; index ++) {

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

        for (index = peerIndex; index < peers.length; index ++) {
            let peer = peers[index];
            if(peer.peerId !== peerId) {
                let newChild = createParticipantElement(peer);
                newChild.classList.add("peer_style_new");
                list.append(newChild);
                childrenNew.push(newChild);
            }
        }

        for (index = 0; index < childrenNew.length; index ++) {
            let child = childrenNew[index];
            window.getComputedStyle(child).opacity;
            childrenNew[index].classList.remove("peer_style_new");
        }

    }

}

function sendFileChunk(chunkInfo) {

    let file = filesMap.get(chunkInfo.clientFileId);
    if(file) {

        var posEnd = chunkInfo.chunkPosition + chunkInfo.chunkSize;
        if(posEnd > file.size) {
            posEnd = file.size;
        }

        let chunk = file.slice(chunkInfo.chunkPosition, posEnd);

        var reader = new FileReader();
        reader.readAsBinaryString(chunk);
        reader.onloadend = function() {

            let data = btoa(reader.result);

            let chunkData = {
                serverFileId: chunkInfo.serverFileId,
                subscriberId: chunkInfo.subscriberId,
                data: data
            }

            let message = {
                peerId: peerId,
                code: 7,
                file: chunkData
            }

            socketSendNextData(JSON.stringify(message));

            let chunkSize = posEnd - chunkInfo.chunkPosition;
            let sentLabel = document.getElementById("file_served_" + chunkInfo.serverFileId);
            let sent = parseInt(sentLabel.getAttribute("amount-sent"));
            let total = sent + chunkSize;
            sentLabel.setAttribute("amount-sent", total.toString());
            sentLabel.textContent = "Sent: " + humanFileSize(total);

        }

    }

}

function handleFiles(files) {

    for(index = 0; index < files.length; index ++ ) {

        let file = files[index];
        let fileId = nextFileId();

        filesMap.set(fileId, file);

        let message = {
            code: 5,
            file: {
                name: file.name,
                clientFileId: fileId,
                size: file.size
            }
        }

        socketSendNextData(JSON.stringify(message));
        document.getElementById('file_share_button').value = "";

    }

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

        case 4: // file
            postSharedFile(message);
            break;

        case 6: // CODE_FILE_REQUEST_CHUNK
            sendFileChunk(message.file);
            break;

    }

}

function socketSendNextData(data) {
    socket.send(data);
}
