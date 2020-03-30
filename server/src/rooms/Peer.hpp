/***************************************************************************
 *
 * Project:   ______                ______ _
 *           / _____)              / _____) |          _
 *          | /      ____ ____ ___| /     | | _   ____| |_
 *          | |     / _  |  _ (___) |     | || \ / _  |  _)
 *          | \____( ( | | | | |  | \_____| | | ( ( | | |__
 *           \______)_||_|_| |_|   \______)_| |_|\_||_|\___)
 *
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef ASYNC_SERVER_ROOMS_PEER_HPP
#define ASYNC_SERVER_ROOMS_PEER_HPP

#include "dto/DTOs.hpp"
#include "rooms/File.hpp"

#include "oatpp-websocket/AsyncWebSocket.hpp"

#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/async/Executor.hpp"

#include "oatpp/core/data/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

class Room; // FWD

class Peer : public oatpp::websocket::AsyncWebSocket::Listener {
private:

  /**
   * Buffer for messages. Needed for multi-frame messages.
   */
  oatpp::data::stream::ChunkedBuffer m_messageBuffer;

  /**
   * Lock for synchronization of writes to the web socket.
   */
  oatpp::async::Lock m_writeLock;

private:
  std::shared_ptr<AsyncWebSocket> m_socket;
  std::shared_ptr<Room> m_room;
  oatpp::String m_nickname;
  v_int64 m_userId;
  std::list<std::shared_ptr<File>> m_files;
private:

  /**
   * Inject async executor object.
   */
  OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
  OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, m_objectMapper);

public:

  Peer(const std::shared_ptr<AsyncWebSocket>& socket,
       const std::shared_ptr<Room>& room,
       const oatpp::String& nickname,
       v_int64 userId)
    : m_socket(socket)
    , m_room(room)
    , m_nickname(nickname)
    , m_userId(userId)
  {}

  /**
   * Send message to peer (to user).
   * @param message
   */
  void sendMessage(const MessageDto::ObjectWrapper& message);

  /**
   * Get room of the peer.
   * @return
   */
  std::shared_ptr<Room> getRoom();

  /**
   * Get peer nickname.
   * @return
   */
  oatpp::String getNickname();

  /**
   * Get peer userId.
   * @return
   */
  v_int64 getUserId();

  /**
   * Add file shared by user. (for indexing purposes)
   */
  void addFile(const std::shared_ptr<File>& file);

  /**
   * List of shared by user files.
   * @return
   */
  const std::list<std::shared_ptr<File>>& getFiles();

  /**
   * Remove circle `std::shared_ptr` dependencies
   */
  void invalidateSocket();

public: // WebSocket Listener methods

  CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
  CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
  CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) override;
  CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

};


#endif //ASYNC_SERVER_ROOMS_PEER_HPP
