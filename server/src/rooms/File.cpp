//
// Created by Leonid  on 23.03.2020.
//

#include "File.hpp"

#include "dto/DTOs.hpp"
#include "rooms/Peer.hpp"

File::Subscriber::Subscriber(v_int64 id, const std::shared_ptr<File>& file)
  : m_id(id)
  , m_file(file)
  , m_valid(true)
  , m_progress(0)
  , m_waitListListener(this)
{
  m_waitList.setListener(&m_waitListListener);
}

File::Subscriber::~Subscriber() {
  m_file->unsubscribe(m_id);
}

void File::Subscriber::requestChunk(v_int64 size) {

  std::lock_guard<std::mutex> lock(m_file->m_subscribersLock);

  if(m_valid) {

    auto message = MessageDto::createShared();
    message->code = MessageCodes::CODE_FILE_REQUEST_CHUNK;

    auto file = FileDto::createShared();
    file->chunkPosition = m_progress;
    file->chunkSize = size;
    file->subscriberId = m_id;

    m_file->m_host->sendMessage(message);

  }

}

oatpp::async::CoroutineStarter File::Subscriber::waitForChunkAsync() {

  class WaitCoroutine : public oatpp::async::Coroutine<WaitCoroutine> {
  private:
    Subscriber* m_subscriber;
  public:

    WaitCoroutine(Subscriber* subscriber)
      : m_subscriber(subscriber)
    {}

    Action act() override {
      std::lock_guard<std::mutex> lock(m_subscriber->m_chunkLock);
      if(m_subscriber->m_chunk) {
        return finish();
      }
      return Action::createWaitListAction(&m_subscriber->m_waitList);
    }

  };

  return WaitCoroutine::start(this);

}

oatpp::v_io_size File::Subscriber::readChunk(void *buffer, v_buff_size count, oatpp::async::Action& action) {

  std::lock_guard<std::mutex> lock(m_chunkLock);

  if(m_progress < m_file->getFileSize()) {

    if (m_chunk) {
      if(m_chunk->getSize() > count) {
        throw std::runtime_error("Invalid chunk size");
      }
      std::memcpy(buffer, m_chunk->getData(), m_chunk->getSize());
      m_progress += m_chunk->getSize();
      return m_chunk->getSize();
    }

    requestChunk(count);
    action = waitForChunkAsync().next(oatpp::async::Action::createActionByType(oatpp::async::Action::TYPE_REPEAT));
    return oatpp::IOError::RETRY_READ;

  }

  return 0;

}

v_int64 File::Subscriber::getId() {
  return m_id;
}

void File::Subscriber::invalidate() {
  m_valid = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File

File::File(const std::shared_ptr<Peer>& host,
           v_int64 clientFileId,
           v_int64 serverFileId,
           const oatpp::String& fileName,
           v_int64 fileSize)
  : m_host(host)
  , m_clientFileId(clientFileId)
  , m_serverFileId(serverFileId)
  , m_fileName(fileName)
  , m_fileSize(fileSize)
  , m_subscriberIdCounter(1)
{}

void File::unsubscribe(v_int64 id) {
  m_subscribers.erase(id);
}

std::shared_ptr<File::Subscriber> File::subscribe() {
  std::lock_guard<std::mutex> lock(m_subscribersLock);
  auto s = std::make_shared<Subscriber>(m_subscriberIdCounter ++, shared_from_this());
  m_subscribers[s->getId()] = s.get();
  return s;
}

v_int64 File::getClientFileId() {
  return m_clientFileId;
}

v_int64 File::getServerFileId() {
  return m_serverFileId;
}

oatpp::String File::getFileName() {
  return m_fileName;
}

v_int64 File::getFileSize() {
  return m_fileSize;
}

void File::clearSubscribers() {
  std::lock_guard<std::mutex> lock(m_subscribersLock);
  for(auto& subscriber : m_subscribers) {
    subscriber.second->invalidate();
  }
  m_subscribers.clear();
}