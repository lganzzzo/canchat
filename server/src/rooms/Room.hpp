
#ifndef ASYNC_SERVER_ROOMS_ROOM_HPP
#define ASYNC_SERVER_ROOMS_ROOM_HPP

#include "./File.hpp"
#include "./Peer.hpp"
#include "dto/DTOs.hpp"

#include <unordered_map>

class Room {
private:
  oatpp::String m_name;
  std::atomic<v_int64> m_fileIdCounter;
  std::unordered_map<v_int64, std::shared_ptr<File>> m_fileById;
  std::unordered_map<v_int64, std::shared_ptr<Peer>> m_peerById;
  std::mutex m_peerByIdLock;
  std::mutex m_fileByIdLock;
public:

  Room(const oatpp::String& name)
    : m_name(name)
    , m_fileIdCounter(1)
  {}

  /**
   * Add peer to the room.
   * @param peer
   */
  void addPeer(const std::shared_ptr<Peer>& peer);

  /**
   * Send room system info to peer.
   * @param peer
   */
  void welcomePeer(const std::shared_ptr<Peer>& peer);

  /**
   * Send peer left room message.
   * @param peer
   */
  void goodbyePeer(const std::shared_ptr<Peer>& peer);

  /**
   * Get peer by id.
   * @param userId
   * @return
   */
  std::shared_ptr<Peer> getPeerById(v_int64 userId);

  /**
   * Remove peer from the room.
   * @param userId
   */
  void removePeerByUserId(v_int64 userId);

  /**
   * Share file.
   * @param hostUserId
   * @param fileClientId
   * @param fileName
   * @param fileSize
   * @return
   */
  std::shared_ptr<File> shareFile(v_int64 hostUserId, v_int64 clientFileId, const oatpp::String& fileName, v_int64 fileSize);

  /**
   * Get file by id.
   * @param fileId
   * @return
   */
  std::shared_ptr<File> getFileById(v_int64 fileId);

  /**
   * Send message to all peers in the room.
   * @param message
   */
  void sendMessage(const MessageDto::ObjectWrapper& message);

};

#endif //ASYNC_SERVER_ROOMS_ROOM_HPP
