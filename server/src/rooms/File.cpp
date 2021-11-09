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

void File::Subscriber::provideFileChunk(const oatpp::String& data) {
  std::lock_guard<std::mutex> lock(m_chunkLock);
  if(m_chunk != nullptr) {
    throw std::runtime_error("File chunk collision.");
  }
  m_chunk = data;
  m_waitList.notifyAll();
}

void File::Subscriber::requestChunk(v_int64 size) {

  if(m_valid) {

    auto message = MessageDto::createShared();
    message->code = MessageCodes::CODE_FILE_REQUEST_CHUNK;

    message->files = MessageDto::FilesList::createShared();

    auto file = FileDto::createShared();
    file->clientFileId = m_file->m_clientFileId;
    file->serverFileId = m_file->m_serverFileId;
    file->subscriberId = m_id;

    file->chunkPosition = m_progress;
    file->chunkSize = size;

    message->files->push_back(file);

    m_file->m_host->sendMessageAsync(message);

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
      if(m_subscriber->m_chunk || !m_subscriber->m_valid) {
        return finish();
      }
      return Action::createWaitListAction(&m_subscriber->m_waitList);
    }

  };

  return WaitCoroutine::start(this);

}

oatpp::v_io_size File::Subscriber::readChunk(void *buffer, v_buff_size count, oatpp::async::Action& action) {

  std::lock_guard<std::mutex> lock(m_chunkLock);

  if(!m_valid) {
    throw std::runtime_error("File is not valid any more.");
  }

  if(m_progress < m_file->getFileSize()) {

    if (m_chunk) {
      v_int64 chunkSize = m_chunk->size();
      if(chunkSize > count) {
        throw std::runtime_error("Invalid chunk size");
      }
      std::memcpy(buffer, m_chunk->data(), chunkSize);
      m_progress += chunkSize;
      m_chunk = nullptr;
      return chunkSize;
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
  std::lock_guard<std::mutex> lock(m_chunkLock);
  m_valid = false;
  m_waitList.notifyAll();
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
  std::lock_guard<std::mutex> lock(m_subscribersLock);
  m_subscribers.erase(id);
}

std::shared_ptr<File::Subscriber> File::subscribe() {
  std::lock_guard<std::mutex> lock(m_subscribersLock);
  auto s = std::make_shared<Subscriber>(m_subscriberIdCounter ++, shared_from_this());
  m_subscribers[s->getId()] = s.get();
  return s;
}

void File::provideFileChunk(v_int64 subscriberId, const oatpp::String& data) {

  std::lock_guard<std::mutex> lock(m_subscribersLock);
  auto it = m_subscribers.find(subscriberId);

  if(it != m_subscribers.end()) {
    it->second->provideFileChunk(data);
  } // else ignore.

}

std::shared_ptr<Peer> File::getHost() {
  return m_host;
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