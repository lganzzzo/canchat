//
// Created by Leonid  on 21.03.2020.
//

#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/core/data/mapping/type/Object.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

enum MessageCodes : v_int64 {

  CODE_INFO = 0,
  CODE_PEER_JOINED = 1,
  CODE_PEER_LEFT = 2,
  CODE_PEER_MESSAGE = 3,
  CODE_PEER_MESSAGE_FILE = 4,

  CODE_FILE_SHARE = 5,
  CODE_FILE_REQUEST_CHUNK = 6,
  CODE_FILE_CHUNK_DATA = 7

};

class PeerDto : public oatpp::data::mapping::type::Object {
public:

  DTO_INIT(PeerDto, Object)

  DTO_FIELD(Int64, peerId);
  DTO_FIELD(String, peerName);

};

class FileDto : public oatpp::data::mapping::type::Object {

  DTO_INIT(FileDto, Object)

  DTO_FIELD(Int64, clientFileId);
  DTO_FIELD(Int64, serverFileId);
  DTO_FIELD(String, name);
  DTO_FIELD(Int64, size);

  DTO_FIELD(Int64, chunkPosition);
  DTO_FIELD(Int64, chunkSize);
  DTO_FIELD(Int64, subscriberId);
  DTO_FIELD(String, data); // base64 data

};

class MessageDto : public oatpp::data::mapping::type::Object {
public:

  DTO_INIT(MessageDto, Object)

  DTO_FIELD(Int64, peerId);
  DTO_FIELD(String, peerName);
  DTO_FIELD(Int32, code);
  DTO_FIELD(String, message);
  DTO_FIELD(Int64, timestamp);
  DTO_FIELD(List<PeerDto::ObjectWrapper>::ObjectWrapper, peers);

  DTO_FIELD(FileDto::ObjectWrapper, file);

};

#include OATPP_CODEGEN_END(DTO)

#endif // DTOs_hpp
