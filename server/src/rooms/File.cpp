//
// Created by Leonid  on 23.03.2020.
//

#include "File.hpp"

File::Subscriber::Subscriber(v_int64 id, File* file)
  : m_id(id)
  , m_file(file)
{}

File::Subscriber::~Subscriber() {
  m_file->unsubscribe(m_id);
}

v_int64 File::Subscriber::getId() {
  return m_id;
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
  auto s = std::make_shared<Subscriber>(m_subscriberIdCounter ++, this);
  m_subscribers[s->getId()] = s;
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