# Room-Based Asynchronous Chat Server

Simple example of a high performance room-based chat server. Built with oat++ (AKA oatpp) web framework using Async API.
Server can handle a large number of simultaneous connections.

See more:

- [Oat++ Website](https://oatpp.io/)
- [Oat++ Github Repository](https://github.com/oatpp/oatpp)
- [Get Started With Oat++](https://oatpp.io/docs/start)

## Overview

User connectes on a websocket endpoint `ws://localhost:8000/ws/chat/{room_name}/?nickname={nickname}` and
enters the chat room `room_name`. User will appear in the chat as a `nickname`.  

If user is the first user in the room,- new room will be automatically created.

Each message that user send to the chat room will be delivered to every participant of the room.

### Project layout

```
|- CMakeLists.txt                         // projects CMakeLists.txt
|- src/
|    |
|    |- controller/
|    |    |
|    |    |- RoomsController.hpp          // ApiController with websocket endpoint
|    |
|    |- rooms/
|    |    |
|    |    |- Lobby.hpp                    // Class for managing new peers and assigning them to rooms
|    |    |- Peer.hpp                     // Class representing one peer of a chat
|    |    |- Room.hpp                     // Class representing one chat room
|    |
|    |- AppComponent.hpp                  // Application config. 
|    |- App.cpp                           // main() is here
|
|- utility/install-oatpp-modules.sh       // utility script to install required oatpp-modules.  
```

## Build and Run

### Using CMake

**Requires:** [oatpp](https://github.com/oatpp/oatpp), and [oatpp-websocket](https://github.com/oatpp/oatpp-websocket) 
modules installed. You may run `utility/install-oatpp-modules.sh` 
script to install required oatpp modules.

After all dependencies satisfied:

```
$ mkdir build && cd build
$ cmake ..
$ make 
$ ./async-server-rooms-exe       # - run application.
```

### In Docker

```
$ docker build -t rooms-server .
$ docker run -p 8000:8000 -t rooms-server
```

## How to test

Browser tab1:

- Goto [http://www.websocket.org/echo.html](http://www.websocket.org/echo.html).
- In the "location" field put - `ws://localhost:8000/ws/chat/Room1/?nickname=Nick`
- Press "Connect" button.
- Verify recieved `RECEIVED: Nick joined Room1`

Browser tab2:

- Goto [http://www.websocket.org/echo.html](http://www.websocket.org/echo.html).
- In the "location" field put - `ws://localhost:8000/ws/chat/Room1/?nickname=Alex`
- Press "Connect" button.
- Verify recieved `RECEIVED: Alex joined Room1`

Now try to send messages from both browser tabs and see that messages are delivered to both participants.

