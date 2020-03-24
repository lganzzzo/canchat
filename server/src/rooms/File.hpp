//
// Created by Leonid  on 23.03.2020.
//

#ifndef File_hpp
#define File_hpp

#include "oatpp/core/data/stream/Stream.hpp"
#include <unordered_map>

class Peer; // FWD

class File {
public:

  class Subscriber {
  private:
    v_int64 m_id;
    File* m_file;
  public:

    Subscriber(v_int64 id, File* file);
    ~Subscriber();

    v_int64 getId();

  };

private:

  void unsubscribe(v_int64 id);

private:

  std::shared_ptr<Peer> m_host;
  v_int64 m_clientFileId;
  v_int64 m_serverFileId;
  oatpp::String m_fileName;
  v_int64 m_fileSize;

  std::atomic<v_int64> m_subscriberIdCounter;
  std::unordered_map<v_int64, std::shared_ptr<Subscriber>> m_subscribers;

public:

  File(const std::shared_ptr<Peer>& host,
       v_int64 clientFileId,
       v_int64 serverFileId,
       const oatpp::String& fileName,
       v_int64 fileSize);

  std::shared_ptr<Subscriber> subscribe();

  v_int64 getClientFileId();

  v_int64 getServerFileId();

  oatpp::String getFileName();

  v_int64 getFileSize();

};


#endif // File_hpp
