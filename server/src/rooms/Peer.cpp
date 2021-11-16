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

#include "Peer.hpp"
#include "Room.hpp"

#include "oatpp/network/tcp/Connection.hpp"
#include "oatpp/encoding/Base64.hpp"

void Peer::sendMessageAsync(const oatpp::Object<MessageDto>& message) {

  class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
    oatpp::String m_message;
  public:

    SendMessageCoroutine(oatpp::async::Lock* lock,
                         const std::shared_ptr<AsyncWebSocket>& websocket,
                         const oatpp::String& message)
      : m_lock(lock)
      , m_websocket(websocket)
      , m_message(message)
    {}

    Action act() override {
      return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
    }

  };

  if(m_socket) {
    m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, m_objectMapper->writeToString(message));
  }

}

bool Peer::sendPingAsync() {

  class SendPingCoroutine : public oatpp::async::Coroutine<SendPingCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
  public:

    SendPingCoroutine(oatpp::async::Lock* lock, const std::shared_ptr<AsyncWebSocket>& websocket)
      : m_lock(lock)
      , m_websocket(websocket)
    {}

    Action act() override {
      return oatpp::async::synchronize(m_lock, m_websocket->sendPingAsync(nullptr)).next(finish());
    }

  };

  /******************************************************
   *
   * Ping counter is increased on sending ping
   * and decreased on receiving pong from the client.
   *
   * If the server didn't receive pong from client
   * before the next ping,- then the client is
   * considered to be disconnected.
   *
   ******************************************************/

  ++ m_pingPoingCounter;

  if(m_socket && m_pingPoingCounter == 1) {
    m_asyncExecutor->execute<SendPingCoroutine>(&m_writeLock, m_socket);
    return true;
  }

  return false;

}

oatpp::async::CoroutineStarter Peer::onApiError(const oatpp::String& errorMessage) {

  class SendErrorCoroutine : public oatpp::async::Coroutine<SendErrorCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
    oatpp::String m_message;
  public:

    SendErrorCoroutine(oatpp::async::Lock* lock,
                       const std::shared_ptr<AsyncWebSocket>& websocket,
                       const oatpp::String& message)
      : m_lock(lock)
      , m_websocket(websocket)
      , m_message(message)
    {}

    Action act() override {
      /* synchronized async pipeline */
      return oatpp::async::synchronize(
        /* Async write-lock to prevent concurrent writes to socket */
        m_lock,
        /* send error message, then close-frame */
        std::move(m_websocket->sendOneFrameTextAsync(m_message).next(m_websocket->sendCloseAsync()))
      ).next(
        /* async error after error message and close-frame are sent */
        new oatpp::async::Error("API Error")
      );
    }

  };

  auto message = MessageDto::createShared();
  message->code = MessageCodes::CODE_API_ERROR;
  message->message = errorMessage;

  return SendErrorCoroutine::start(&m_writeLock, m_socket, m_objectMapper->writeToString(message));

}

oatpp::async::CoroutineStarter Peer::validateFilesList(const MessageDto::FilesList& filesList) {

  if(filesList->size() == 0)
    return onApiError("Files list is empty.");

  for(auto& fileDto : *filesList) {

    if (!fileDto)
      return onApiError("File structure is not provided.");
    if (!fileDto->clientFileId)
      return onApiError("File clientId is not provided.");
    if (!fileDto->name)
      return onApiError("File name is not provided.");
    if (!fileDto->size)
      return onApiError("File size is not provided.");

  }

  return nullptr;

}

oatpp::async::CoroutineStarter Peer::handleFilesMessage(const oatpp::Object<MessageDto>& message) {

  auto files = message->files;
  validateFilesList(files);

  auto fileMessage = MessageDto::createShared();
  fileMessage->code = MessageCodes::CODE_PEER_MESSAGE_FILE;
  fileMessage->peerId = m_peerId;
  fileMessage->peerName = m_nickname;
  fileMessage->timestamp = oatpp::base::Environment::getMicroTickCount();
  fileMessage->files = MessageDto::FilesList::createShared();

  for(auto& currFile : *files) {

    auto file = m_room->shareFile(m_peerId, currFile->clientFileId, currFile->name, currFile->size);

    auto sharedFile = FileDto::createShared();
    sharedFile->serverFileId = file->getServerFileId();
    sharedFile->name = file->getFileName();
    sharedFile->size = file->getFileSize();

    fileMessage->files->push_back(sharedFile);

  }

  m_room->addHistoryMessage(fileMessage);
  m_room->sendMessageAsync(fileMessage);

  return nullptr;

}

oatpp::async::CoroutineStarter Peer::handleFileChunkMessage(const oatpp::Object<MessageDto>& message) {

  auto filesList = message->files;
  if(!filesList)
    return onApiError("No file provided.");

  if(filesList->size() > 1)
    return onApiError("Invalid files count. Expected - 1.");

  auto fileDto = filesList->front();
  if (!fileDto)
    return onApiError("File structure is not provided.");
  if (!fileDto->serverFileId)
    return onApiError("File clientId is not provided.");
  if (!fileDto->subscriberId)
    return onApiError("File subscriberId is not provided.");
  if (!fileDto->data)
    return onApiError("File chunk data is not provided.");

  auto file = m_room->getFileById(fileDto->serverFileId);

  if(!file) return nullptr; // Ignore if file doesn't exist. File may be deleted already.

  if(file->getHost()->getPeerId() != getPeerId())
    return onApiError("Wrong file host.");

  auto data = oatpp::encoding::Base64::decode(fileDto->data);
  file->provideFileChunk(fileDto->subscriberId, data);

  return nullptr;

}

oatpp::async::CoroutineStarter Peer::handleMessage(const oatpp::Object<MessageDto>& message) {

  if(!message->code) {
    return onApiError("No message code provided.");
  }

  switch(*message->code) {

    case MessageCodes::CODE_PEER_MESSAGE:
      m_room->addHistoryMessage(message);
      m_room->sendMessageAsync(message);
      ++ m_statistics->EVENT_PEER_SEND_MESSAGE;
      break;

    case MessageCodes::CODE_PEER_IS_TYPING:
      m_room->sendMessageAsync(message); break;

    case MessageCodes::CODE_FILE_SHARE:
      return handleFilesMessage(message);

    case MessageCodes::CODE_FILE_CHUNK_DATA:
      return handleFileChunkMessage(message);

    default:
      return onApiError("Invalid client message code.");

  }

  return nullptr;

}

std::shared_ptr<Room> Peer::getRoom() {
  return m_room;
}

oatpp::String Peer::getNickname() {
  return m_nickname;
}

v_int64 Peer::getPeerId() {
  return m_peerId;
}

void Peer::addFile(const std::shared_ptr<File>& file) {
  m_files.push_back(file);
}

const std::list<std::shared_ptr<File>>& Peer::getFiles() {
  return m_files;
}

void Peer::invalidateSocket() {
  if(m_socket) {
    m_socket->getConnection().invalidate();
  }
  m_socket.reset();
}

oatpp::async::CoroutineStarter Peer::onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter Peer::onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  -- m_pingPoingCounter;
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter Peer::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) {
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter Peer::readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

  if(m_messageBuffer.getCurrentPosition() + size >  m_appConfig->maxMessageSizeBytes) {
    return onApiError("Message size exceeds max allowed size.");
  }

  if(size == 0) { // message transfer finished

    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.setCurrentPosition(0);

    oatpp::Object<MessageDto> message;

    try {
      message = m_objectMapper->readFromString<oatpp::Object<MessageDto>>(wholeMessage);
    } catch (const std::runtime_error& e) {
      return onApiError("Can't parse message");
    }

    message->peerName = m_nickname;
    message->peerId = m_peerId;
    message->timestamp = oatpp::base::Environment::getMicroTickCount();

    return handleMessage(message);

  } else if(size > 0) { // message frame received
    m_messageBuffer.writeSimple(data, size);
  }

  return nullptr; // do nothing

}