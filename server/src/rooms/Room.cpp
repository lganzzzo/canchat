
#include "Room.hpp"

void Room::addPeer(const std::shared_ptr<Peer>& peer) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  m_peerById[peer->getUserId()] = peer;
}

void Room::welcomePeer(const std::shared_ptr<Peer>& peer) {

  auto infoMessage = MessageDto::createShared();
  infoMessage->code = MessageCodes::CODE_INFO;
  infoMessage->peerId = peer->getUserId();
  infoMessage->peerName = peer->getNickname();

  infoMessage->peers = infoMessage->peers->createShared();

  for(auto& it : m_peerById) {
    auto p = PeerDto::createShared();
    p->peerId = it.second->getUserId();
    p->peerName = it.second->getNickname();
    infoMessage->peers->pushBack(p);
  }

  peer->sendMessage(infoMessage);

  auto joinedMessage = MessageDto::createShared();
  joinedMessage->code = MessageCodes::CODE_PEER_JOINED;
  joinedMessage->peerId = peer->getUserId();
  joinedMessage->peerName = peer->getNickname();
  joinedMessage->message = "'" + peer->getNickname() + "' joined room";

  sendMessage(joinedMessage);
}

void Room::goodbyePeer(const std::shared_ptr<Peer>& peer) {

  auto message = MessageDto::createShared();
  message->code = MessageCodes::CODE_PEER_LEFT;
  message->peerId = peer->getUserId();
  message->message = "'" + peer->getNickname() + "' left room";

  sendMessage(message);

}

std::shared_ptr<Peer> Room::getPeerById(v_int64 userId) {
  auto it = m_peerById.find(userId);
  if(it != m_peerById.end()) {
    return it->second;
  }
  return nullptr;
}

void Room::removePeerByUserId(v_int64 userId) {

  auto peer = getPeerById(userId);
  if(peer) {

    {
      std::lock_guard<std::mutex> guard(m_fileByIdLock);
      for (const auto &file : peer->getFiles()) {
        file->clearSubscribers();
        m_fileById.erase(file->getServerFileId());
      }

    }

    {
      std::lock_guard<std::mutex> guard(m_peerByIdLock);
      m_peerById.erase(userId);
    }

  }

}

std::shared_ptr<File> Room::shareFile(v_int64 hostUserId, v_int64 clientFileId, const oatpp::String& fileName, v_int64 fileSize) {
  std::lock_guard<std::mutex> guard(m_fileByIdLock);
  auto host = getPeerById(hostUserId);
  if(!host) throw std::runtime_error("File host not found.");
  v_int64 serverFileId = m_fileIdCounter ++;
  auto file = std::make_shared<File>(host, clientFileId, serverFileId, fileName, fileSize);
  host->addFile(file);
  m_fileById[serverFileId] = file;
  return file;
}

std::shared_ptr<File> Room::getFileById(v_int64 fileId) {
  std::lock_guard<std::mutex> guard(m_fileByIdLock);
  auto it = m_fileById.find(fileId);
  if(it != m_fileById.end()) {
    return it->second;
  }
  return nullptr;
}

void Room::sendMessage(const MessageDto::ObjectWrapper& message) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  for(auto& pair : m_peerById) {
    pair.second->sendMessage(message);
  }
}
