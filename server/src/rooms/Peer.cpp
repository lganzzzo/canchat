
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

  m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, m_objectMapper->writeToString(message));

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
    message->timestamp = oatpp::base::Environment::getMicroTickCount();

    if(!message->code) {
      throw std::runtime_error("No message code provided.");
    }

    switch(message->code->getValue()) {

      case MessageCodes::CODE_PEER_JOINED: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_LEFT: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_MESSAGE: m_room->sendMessage(message); break;
      case MessageCodes::CODE_PEER_MESSAGE_FILE: m_room->sendMessage(message); break;
      case MessageCodes::CODE_FILE_SHARE:
        {
          auto fileDto = message->file;

          if (!fileDto) throw std::runtime_error("File structure is not provided.");
          if (!fileDto->clientFileId) throw std::runtime_error("File clientId is not provided.");
          if (!fileDto->name) throw std::runtime_error("File name is not provided.");
          if (!fileDto->size) throw std::runtime_error("File size is not provided.");

          auto file = m_room->shareFile(m_userId, fileDto->clientFileId->getValue(), fileDto->name, fileDto->size->getValue());

          auto fileMessage = MessageDto::createShared();
          fileMessage->code = MessageCodes::CODE_PEER_MESSAGE_FILE;
          fileMessage->peerId = m_userId;
          fileMessage->peerName = m_nickname;
          fileMessage->timestamp = oatpp::base::Environment::getMicroTickCount();

          auto sharedFile = FileDto::createShared();
          sharedFile->serverFileId = file->getServerFileId();
          sharedFile->name = file->getFileName();
          sharedFile->size = file->getFileSize();

          fileMessage->file = sharedFile;

          m_room->sendMessage(fileMessage);

        }
        break;

      case MessageCodes::CODE_FILE_CHUNK_DATA:
        {
          auto fileDto = message->file;
          if (!fileDto) throw std::runtime_error("File structure is not provided.");
          if (!fileDto->serverFileId) throw std::runtime_error("File clientId is not provided.");
          if (!fileDto->subscriberId) throw std::runtime_error("File subscriberId is not provided.");
          if (!fileDto->data) throw std::runtime_error("File chunk data is not provided.");

          auto file = m_room->getFileById(fileDto->serverFileId->getValue());
          if(!file) break; // Ignore if file doesn't exist. File may be deleted already.
          if(file->getHost()->getUserId() != getUserId()) throw std::runtime_error("Wrong file host.");

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