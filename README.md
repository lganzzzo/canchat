# Can-Chat [![Build Status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/lganzzzo.canchat?branchName=master)](https://dev.azure.com/lganzzzo/lganzzzo/_build/latest?definitionId=24&branchName=master)

![Can-Chat](https://upload.wikimedia.org/wikipedia/commons/0/0a/Tr%C3%A5dtelefon-illustration.png)

Feature-complete rooms-based chat for tens of thousands users. Client plus Server. Built with [Oat++ Web Framework](https://oatpp.io/).   
`Single-Node` `In-Memory` `WebSocket` `TLS`.


## Quick Start

- Run your own chat in Docker:
   ```bash
   docker run -p 8443:8443 -e EXTERNAL_ADDRESS=localhost -e EXTERNAL_PORT=8443 -it lganzzzo/canchat
   ```
- [Build from source](#build-from-source)

## Chat

### Rooms

All communication happens in chat-rooms.  
Each room has a unique URL.  
Anyone who has room-URL can join the room.
Generating random long enough room-URL gives privacy to participants. Be sure to share room-URL with wanted participants only. 

Rooms are automatically created when the first participant joins the room, and
are automatically deleted when the last participant leaves the room.

Chat-history is available for newly joined participants.
History is rounded when the number of messages in the room exceeds the `maxRoomHistoryMessages` config parameter.
History is deleted and no-longer-available once the room is deleted.

### File Sharing

Users can share multiple files at once.

Files are streamed directly from the host-peer machine. 
Nothing is uploaded to the server nor stored in any 3d-party place.

Once host-peer cancels file-sharing, files are no longer available.  
If host-peer exits the room, all of his file-sharing is canceled.


## Build And Run

### Build From Source

#### Pre-Requirements

- Install `LibreSSL` version `3.0.0` or later.
- Install [oatpp](https://github.com/oatpp/oatpp) - Oat++ Web Framework (main module)
- Install [oatpp-websocket](https://github.com/oatpp/oatpp-websocket) - Oat++ WebSocket extension module.
- Install [oatpp-libressl](https://github.com/oatpp/oatpp-libressl) - Libressl adaptor for Oat++.

*To install oatpp modules you can use `utility/install-oatpp-modules.sh` script*

#### Build Chat Server

```bash
$ cd <this-repo>/server/
$ mkdir build/ && cd build/
$ cmake ..
$ make
```

Now run chat server

```bash
$ canchat-exe --host localhost --port 8443
```

### Docker

```bash
$ docker build -t canchat .
$ docker run -p 8443:8443 -e EXTERNAL_ADDRESS=localhost -e EXTERNAL_PORT=8443 -it canchat
```
*Then goto https://localhost:8443/ in browser. (accept self-signed certificate)*

Image parameters:
- `-e EXTERNAL_ADDRESS` - Address to which the client is supposed to connect to.
- `-e EXTERNAL_PORT` - Port at `EXTERNAL_ADDRESS` to which the client is supposed to connect to. *Chat server will also be running on this port*.
- `-e TLS_FILE_PRIVATE_KEY` - Path to TLS private-key file.
- `-e TLS_FILE_CERT_CHAIN` - Path to TLS certificate chain file.

## Repository Content

```
- front/                            # - Front-end sources are here
- server/                           # - Server sources are here
- cert/                             # - test TLS certificate is here
- utility/
    |
    |- install-oatpp-modules.sh     # - useful script to install required oatpp modules
```

## License

This project is released under `Apache License 2.0`.

---

Enjoy!
