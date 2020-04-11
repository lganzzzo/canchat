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
  CODE_PEER_IS_TYPING = 5,

  CODE_FILE_SHARE = 6,
  CODE_FILE_REQUEST_CHUNK = 7,
  CODE_FILE_CHUNK_DATA = 8,

  CODE_API_ERROR = 9

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
  typedef List<FileDto::ObjectWrapper> FilesList;
public:

  DTO_INIT(MessageDto, Object)

  DTO_FIELD(Int64, peerId);
  DTO_FIELD(String, peerName);
  DTO_FIELD(Int32, code);
  DTO_FIELD(String, message);
  DTO_FIELD(Int64, timestamp);

  DTO_FIELD(List<PeerDto::ObjectWrapper>::ObjectWrapper, peers);
  DTO_FIELD(List<MessageDto::ObjectWrapper>::ObjectWrapper, history);

  DTO_FIELD(List<FileDto::ObjectWrapper>::ObjectWrapper, files);

};

#include OATPP_CODEGEN_END(DTO)

#endif // DTOs_hpp
