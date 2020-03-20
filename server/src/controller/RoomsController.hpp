
#ifndef RoomsController_hpp
#define RoomsController_hpp

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/network/server/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) /// <-- Begin Code-Gen

class RoomsController : public oatpp::web::server::api::ApiController {
private:
  typedef RoomsController __ControllerType;
private:
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, websocketConnectionHandler, "websocket");
public:
  RoomsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

  ENDPOINT_ASYNC("GET", "api/ws/room/{roomId}/", WS) {

    ENDPOINT_ASYNC_INIT(WS)

    Action act() override {

      auto roomName = request->getPathVariable("roomId");
      auto nickname = "Mr.X";

      OATPP_ASSERT_HTTP(nickname, Status::CODE_400, "No nickname specified.");

      /* Websocket handshake */
      auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);

      auto parameters = std::make_shared<oatpp::network::server::ConnectionHandler::ParameterMap>();

      (*parameters)["roomName"] = roomName;
      (*parameters)["nickname"] = nickname;

      /* Set connection upgrade params */
      response->setConnectionUpgradeParameters(parameters);

      return _return(response);

    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif /* RoomsController_hpp */
