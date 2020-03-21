
#ifndef StaticController_hpp
#define StaticController_hpp

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include <regex>

#include OATPP_CODEGEN_BEGIN(ApiController) /// <-- Begin Code-Gen

class StaticController : public oatpp::web::server::api::ApiController {
private:
  typedef StaticController __ControllerType;
private:
  static oatpp::String loadFile(const char* filename) {
    auto buffer = oatpp::base::StrBuffer::loadFromFile(filename);
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
      static auto fileCache = loadFile(FRONT_PATH "/index.html");
      auto response = controller->createResponse(Status::CODE_200, fileCache);
      response->putHeader(Header::CONTENT_TYPE, "text/html");
      return _return(response);
    }

  };

  ENDPOINT_ASYNC("GET", "room/{roomId}", ChatHTML) {

    ENDPOINT_ASYNC_INIT(ChatHTML)

    Action act() override {
      /*static*/ auto fileCache = loadFile(FRONT_PATH "/chat/index.html")->std_str();
      auto text = std::regex_replace(fileCache, std::regex("%%%ROOM_ID%%%"), request->getPathVariable("roomId")->std_str());
      auto response = controller->createResponse(Status::CODE_200, oatpp::String(text.data(), text.size(), true));
      response->putHeader(Header::CONTENT_TYPE, "text/html");
      return _return(response);
    }

  };

  ENDPOINT_ASYNC("GET", "room/{roomId}/chat.js", ChatJS) {

    ENDPOINT_ASYNC_INIT(ChatJS)

    Action act() override {
      /*static*/ auto fileCache = loadFile(FRONT_PATH "/chat/chat.js")->std_str();
      oatpp::String url = "wss://" CHAT_HOST "/api/ws/room/" + request->getPathVariable("roomId");
      auto text = std::regex_replace(fileCache, std::regex("%%%URL%%%"), url->std_str());
      auto response = controller->createResponse(Status::CODE_200, oatpp::String(text.data(), text.size(), true));
      response->putHeader(Header::CONTENT_TYPE, "text/javascript");
      return _return(response);
    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif // StaticController_hpp
