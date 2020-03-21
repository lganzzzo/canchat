//
// Created by Leonid  on 21.03.2020.
//

#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/core/data/mapping/type/Object.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class MessageDto : public oatpp::data::mapping::type::Object {
public:

  static const v_int32 CODE_PEER_INFO;
  static const v_int32 CODE_PEER_JOINED;
  static const v_int32 CODE_PEER_LEFT;
  static const v_int32 CODE_PEER_MESSAGE;

  DTO_INIT(MessageDto, Object)

  DTO_FIELD(Int64, peerId);
  DTO_FIELD(String, peerName);
  DTO_FIELD(Int32, code);
  DTO_FIELD(String, message);

};

#include OATPP_CODEGEN_END(DTO)

#endif // DTOs_hpp
