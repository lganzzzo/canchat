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

#ifndef StaticController_hpp
#define StaticController_hpp

#include "dto/Config.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include <regex>

#include OATPP_CODEGEN_BEGIN(ApiController) /// <-- Begin Code-Gen

class StaticController : public oatpp::web::server::api::ApiController {
private:
  typedef StaticController __ControllerType;
private:
  OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_config);
  OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);
private:

  static oatpp::String loadFile(const char* filename) {
    auto buffer = oatpp::String::loadFromFile(filename);
    OATPP_ASSERT_HTTP(buffer, Status::CODE_404, "File Not Found:(");
    return buffer;
  }
public:
  StaticController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

  ENDPOINT_ASYNC("GET", "/", Root) {

    ENDPOINT_ASYNC_INIT(Root)

    Action act() override {
      ++ controller->m_statistics->EVENT_FRONT_PAGE_LOADED;
      static auto fileCache = loadFile(FRONT_PATH "/index.html");
      auto response = controller->createResponse(Status::CODE_200, fileCache);
      response->putHeader(Header::CONTENT_TYPE, "text/html");
      return _return(response);
    }

  };

  ENDPOINT_ASYNC("GET", "room/{roomId}", ChatHTML) {

    ENDPOINT_ASYNC_INIT(ChatHTML)

    Action act() override {
      std::string fileCache = *loadFile(FRONT_PATH "/chat/index.html");
      auto text = std::regex_replace(fileCache, std::regex("%%%ROOM_ID%%%"), *request->getPathVariable("roomId"));
      auto response = controller->createResponse(Status::CODE_200, text);
      response->putHeader(Header::CONTENT_TYPE, "text/html");
      return _return(response);
    }

  };

  ENDPOINT_ASYNC("GET", "room/{roomId}/chat.js", ChatJS) {

    ENDPOINT_ASYNC_INIT(ChatJS)

    Action act() override {
      static auto fileCache = loadFile(FRONT_PATH "/chat/chat.js");

      oatpp::data::stream::BufferOutputStream stream;

      auto baseUrl = controller->m_config->getWebsocketBaseUrl();

      stream << "let urlWebsocket = \"" << baseUrl << "/api/ws/room/" << request->getPathVariable("roomId") << "\";\n";
      stream << "let urlRoom = \"/room/" << request->getPathVariable("roomId") << "\";\n";
      stream << "\n";

      stream << fileCache;

      auto response = controller->createResponse(Status::CODE_200, stream.toString());
      response->putHeader(Header::CONTENT_TYPE, "text/javascript");
      return _return(response);
    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif // StaticController_hpp
