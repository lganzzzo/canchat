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

#include "oatpp/encoding/Base64.hpp"

void Peer::sendMessage(const MessageDto::ObjectWrapper& message) {

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

void Peer::validateFilesList(const MessageDto::FilesList::ObjectWrapper& filesList) {

  auto curr = filesList->getFirstNode();

  if(!curr)
    throw std::runtime_error("Files list is empty.");

  while(curr != nullptr) {

    auto fileDto = curr->getData();

    if (!fileDto)
      throw std::runtime_error("File structure is not provided.");
    if (!fileDto->clientFileId)
      throw std::runtime_error("File clientId is not provided.");
    if (!fileDto->name)
      throw std::runtime_error("File name is not provided.");
    if (!fileDto->size)
      throw std::runtime_error("File size is not provided.");

    curr = curr->getNext();

  }

}

std::shared_ptr<Room> Peer::getRoom() {
  return m_room;
}

oatpp::String Peer::getNickname() {
  return m_nickname;
}

v_int64 Peer::getUserId() {
  return m_userId;
}

void Peer::addFile(const std::shared_ptr<File>& file) {
  m_files.push_back(file);
}

const std::list<std::shared_ptr<File>>& Peer::getFiles() {
  return m_files;
}

void Peer::invalidateSocket() {
  m_socket.reset();
}

oatpp::async::CoroutineStarter Peer::onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter Peer::onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter Peer::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) {
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter Peer::readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

  if(size == 0) { // message transfer finished

    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.clear();

    auto message = m_objectMapper->readFromString<MessageDto>(wholeMessage);
    message->peerName = m_nickname;
    message->peerId = m_userId;
    message->timestamp = oatpp::base::Environment::getMicroTickCount();

    if(!message->code) {
      throw std::runtime_error("No message code provided.");
    }

    switch(message->code->getValue()) {

      case MessageCodes::CODE_PEER_JOINED: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_LEFT: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_MESSAGE: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_MESSAGE_FILE: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_IS_TYPING: m_room->sendMessage(message); break;
      case MessageCodes::CODE_FILE_SHARE:
        {
          auto files = message->files;
          validateFilesList(files);

          auto fileMessage = MessageDto::createShared();
          fileMessage->code = MessageCodes::CODE_PEER_MESSAGE_FILE;
          fileMessage->peerId = m_userId;
          fileMessage->peerName = m_nickname;
          fileMessage->timestamp = oatpp::base::Environment::getMicroTickCount();
          fileMessage->files = MessageDto::FilesList::createShared();

          auto curr = files->getFirstNode();
          while(curr) {

            auto currFile = curr->getData();

            auto file = m_room->shareFile(m_userId, currFile->clientFileId->getValue(), currFile->name, currFile->size->getValue());

            auto sharedFile = FileDto::createShared();
            sharedFile->serverFileId = file->getServerFileId();
            sharedFile->name = file->getFileName();
            sharedFile->size = file->getFileSize();

            fileMessage->files->pushBack(sharedFile);

            curr = curr->getNext();

          }

          m_room->sendMessage(fileMessage);

        }
        break;

      case MessageCodes::CODE_FILE_CHUNK_DATA:
        {
          auto filesList = message->files;
          if(!filesList)
            throw std::runtime_error("No file provided.");

          if(filesList->count() > 1)
            throw std::runtime_error("Invalid files count. Expected - 1.");

          auto fileDto = filesList->getFirst();
          if (!fileDto)
            throw std::runtime_error("File structure is not provided.");
          if (!fileDto->serverFileId)
            throw std::runtime_error("File clientId is not provided.");
          if (!fileDto->subscriberId)
            throw std::runtime_error("File subscriberId is not provided.");
          if (!fileDto->data)
            throw std::runtime_error("File chunk data is not provided.");

          auto file = m_room->getFileById(fileDto->serverFileId->getValue());
          if(!file) break; // Ignore if file doesn't exist. File may be deleted already.
          if(file->getHost()->getUserId() != getUserId())
            throw std::runtime_error("Wrong file host.");

          auto data = oatpp::encoding::Base64::decode(fileDto->data);
          file->provideFileChunk(fileDto->subscriberId->getValue(), data);

        }
        break;

      default:
        throw std::runtime_error("Invalid client message code.");
    }

  } else if(size > 0) { // message frame received
    m_messageBuffer.writeSimple(data, size);
  }

  return nullptr; // do nothing

}