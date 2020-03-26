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

#ifndef File_hpp
#define File_hpp

#include "oatpp/core/data/stream/Stream.hpp"
#include "oatpp/core/async/CoroutineWaitList.hpp"
#include <unordered_map>

class Peer; // FWD

class File : public std::enable_shared_from_this<File> {
public:

  class Subscriber {
  private:

    class WaitListListener : public oatpp::async::CoroutineWaitList::Listener {
    private:
      Subscriber* m_subscriber;
    public:

      WaitListListener(Subscriber* subscriber)
        : m_subscriber(subscriber)
      {}

      void onNewItem(oatpp::async::CoroutineWaitList& list) override {
        std::lock_guard<std::mutex> lock(m_subscriber->m_chunkLock);
        if (m_subscriber->m_chunk || !m_subscriber->m_valid) {
          list.notifyAll();
        }
      }

    };

  private:
    v_int64 m_id;
    std::shared_ptr<File> m_file;
    bool m_valid;
    v_int64 m_progress;
  private:
    std::mutex m_chunkLock;
    oatpp::String m_chunk;
    WaitListListener m_waitListListener;
    oatpp::async::CoroutineWaitList m_waitList;
  private:
    void requestChunk(v_int64 size);
    oatpp::async::CoroutineStarter waitForChunkAsync();
  public:

    Subscriber(v_int64 id, const std::shared_ptr<File>& file);

    ~Subscriber();

    void provideFileChunk(const oatpp::String& data);

    oatpp::v_io_size readChunk(void *buffer, v_buff_size count, oatpp::async::Action& action);

    v_int64 getId();

    void invalidate();

  };

private:

  void unsubscribe(v_int64 id);

private:

  std::shared_ptr<Peer> m_host;
  v_int64 m_clientFileId;
  v_int64 m_serverFileId;
  oatpp::String m_fileName;
  v_int64 m_fileSize;

  std::mutex m_subscribersLock;
  std::atomic<v_int64> m_subscriberIdCounter;
  std::unordered_map<v_int64, Subscriber*> m_subscribers;

public:

  File(const std::shared_ptr<Peer>& host,
       v_int64 clientFileId,
       v_int64 serverFileId,
       const oatpp::String& fileName,
       v_int64 fileSize);

  std::shared_ptr<Subscriber> subscribe();

  void provideFileChunk(v_int64 subscriberId, const oatpp::String& data);

  std::shared_ptr<Peer> getHost();

  v_int64 getClientFileId();

  v_int64 getServerFileId();

  oatpp::String getFileName();

  v_int64 getFileSize();

  void clearSubscribers();

};


#endif // File_hpp
