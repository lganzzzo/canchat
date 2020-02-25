
#include "Room.hpp"

void Room::addPeer(const std::shared_ptr<Peer>& peer) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  m_peerById[peer->getUserId()] = peer;
}

void Room::removePeerByUserId(v_int32 userId) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  m_peerById.erase(userId);
}

void Room::sendMessage(const oatpp::String& message) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  for(auto& pair : m_peerById) {
    pair.second->sendMessage(message);
  }
}