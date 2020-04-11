# Can-Chat [![Build Status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/lganzzzo.canchat?branchName=master)](https://dev.azure.com/lganzzzo/lganzzzo/_build/latest?definitionId=24&branchName=master)

Full-stack (Client + Server) anonymous and secure chat.  

## Rooms

All communication happens in chat-rooms.  
Each room has a unique URL.  
Anyone who has room-URL can join the room.
Generating random long enough room-URL gives privacy to participants. Be sure to share room-URL with wanted participants only. 

Rooms are automatically created when the first participant joins the room, and
are automatically deleted when the last participant leaves the room.

No history is maintained within the room.  
No chat-history is available for newly joined participants.

## File Sharing

Files are streamed directly from the host-peer machine. 
Nothing is uploaded to the server nor stored in any 3d-party place.

Once host-peer cancels file-sharing, files are no longer available.  
If host-peer exits the room, all of its file-sharing is canceled.



