
#ifndef ASYNC_SERVER_ROOMS_ROOM_HPP
#define ASYNC_SERVER_ROOMS_ROOM_HPP

#include "./Peer.hpp"
#include "dto/DTOs.hpp"

#include <unordered_map>

class Room {
private:
  oatpp::String m_name;
  std::unordered_map<v_int64, std::shared_ptr<Peer>> m_peerById;
  std::mutex m_peerByIdLock;
public:

  Room(const oatpp::String& name)
    : m_name(name)
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
   * Remove peer from the room.
   * @param userId
   */
  void removePeerByUserId(v_int64 userId);

  /**
   * Send message to all peers in the room.
   * @param message
   */
  void sendMessage(const MessageDto::ObjectWrapper& message);

};

#endif //ASYNC_SERVER_ROOMS_ROOM_HPP
