
#ifndef StaticController_hpp
#define StaticController_hpp

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

class StaticController : public oatpp::web::server::api::ApiController {
private:
  typedef StaticController __ControllerType;
public:
  StaticController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

/**
 *  Begin ENDPOINTs generation ('ApiController' codegen)
 */
#include OATPP_CODEGEN_BEGIN(ApiController)

  ENDPOINT_ASYNC("GET", "/", Root) {

    ENDPOINT_ASYNC_INIT(Root)

    Action act() override {
      static oatpp::String fileCache = oatpp::base::StrBuffer::loadFromFile(FRONT_PATH "/index.html");
      OATPP_ASSERT_HTTP(fileCache, Status::CODE_404, "File Not Found:(");
      auto response = controller->createResponse(Status::CODE_200, fileCache);
      response->putHeader(Header::CONTENT_TYPE, "text/html");
      return _return(response);
    }

  };

  ENDPOINT_ASYNC("GET", "chat/", ChatHTML) {

  ENDPOINT_ASYNC_INIT(ChatHTML)

    Action act() override {
      static oatpp::String fileCache = oatpp::base::StrBuffer::loadFromFile(FRONT_PATH "/chat/index.html");
      OATPP_ASSERT_HTTP(fileCache, Status::CODE_404, "File Not Found:(");
      auto response = controller->createResponse(Status::CODE_200, fileCache);
      response->putHeader(Header::CONTENT_TYPE, "text/html");
      return _return(response);
    }

  };

  ENDPOINT_ASYNC("GET", "chat/chat.js", ChatJS) {

    ENDPOINT_ASYNC_INIT(ChatJS)

    Action act() override {
      static oatpp::String fileCache = oatpp::base::StrBuffer::loadFromFile(FRONT_PATH "/chat/chat.js");
      OATPP_ASSERT_HTTP(fileCache, Status::CODE_404, "File Not Found:(");
      auto response = controller->createResponse(Status::CODE_200, fileCache);
      response->putHeader(Header::CONTENT_TYPE, "text/javascript");
      return _return(response);
    }

  };

/**
 *  Finish ENDPOINTs generation ('ApiController' codegen)
 */
#include OATPP_CODEGEN_END(ApiController)

};

#endif // StaticController_hpp
