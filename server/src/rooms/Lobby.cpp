
#include "Lobby.hpp"

v_int32 Lobby::obtainNewUserId() {
  return m_userIdCounter ++;
}

std::shared_ptr<Room> Lobby::getOrCreateRoom(const oatpp::String& roomName) {
  std::lock_guard<std::mutex> lock(m_roomsMutex);
  std::shared_ptr<Room>& room = m_rooms[roomName];
  if(!room) {
    room = std::make_shared<Room>(roomName);
  }
  return room;
}

void Lobby::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) {

  auto roomName = params->find("roomName")->second;
  auto nickname = params->find("nickname")->second;
  auto room = getOrCreateRoom(roomName);

  auto peer = std::make_shared<Peer>(socket, room, nickname, obtainNewUserId());
  socket->setListener(peer);

  room->addPeer(peer);
  room->sendMessage(nickname + " joined " + roomName);

}

void Lobby::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket) {

  auto peer = std::static_pointer_cast<Peer>(socket->getListener());
  auto nickname = peer->getNickname();
  auto room = peer->getRoom();

  room->removePeerByUserId(peer->getUserId());

  room->sendMessage(nickname + " left the room");

  /* Remove circle `std::shared_ptr` dependencies */
  socket->setListener(nullptr);

}