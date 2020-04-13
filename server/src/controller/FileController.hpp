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

#ifndef FileController_hpp
#define FileController_hpp

#include "rooms/Lobby.hpp"
#include "utils/Statistics.hpp"

#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include <regex>

#include OATPP_CODEGEN_BEGIN(ApiController) /// <-- Begin Code-Gen

class FileController : public oatpp::web::server::api::ApiController {
private:
  typedef FileController __ControllerType;
private:
  OATPP_COMPONENT(std::shared_ptr<Lobby>, lobby);
public:
  FileController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

  ENDPOINT_ASYNC("GET", "room/{roomId}/file/{fileId}", GetFile) {

    ENDPOINT_ASYNC_INIT(GetFile)

    class ReadCallback : public oatpp::data::stream::ReadCallback {
    private:
      std::shared_ptr<File::Subscriber> m_subscriber;
    private:
      OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);
    public:

      ReadCallback(const std::shared_ptr<File::Subscriber>& subscriber)
        : m_subscriber(subscriber)
      {}

      oatpp::v_io_size read(void *buffer, v_buff_size count, oatpp::async::Action& action) override {
        auto res = m_subscriber->readChunk(buffer, count, action);
        if(res > 0) {
          m_statistics->FILE_SERVED_BYTES += (v_uint64) res;
        }
        return res;
      }

    };

    Action act() override {

      oatpp::String roomId = request->getPathVariable("roomId");

      bool success;
      v_int64 fileId = oatpp::utils::conversion::strToInt64(request->getPathVariable("fileId"), success);
      OATPP_ASSERT_HTTP(success, Status::CODE_400, "Invalid fileId");

      auto room = controller->lobby->getRoom(roomId);
      OATPP_ASSERT_HTTP(room, Status::CODE_404, "Room not found");

      auto file = room->getFileById(fileId);
      OATPP_ASSERT_HTTP(file, Status::CODE_404, "File not found");

      auto body = std::make_shared<oatpp::web::protocol::http::outgoing::StreamingBody>
        (std::make_shared<ReadCallback>(file->subscribe()));

      auto response = OutgoingResponse::createShared(Status::CODE_200, body);
      response->putHeader("content-disposition", "attachment; filename=\"" + file->getFileName() +"\"");

      return _return(response);
    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif // FileController_hpp
