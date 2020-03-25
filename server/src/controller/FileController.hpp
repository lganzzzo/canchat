
#ifndef FileController_hpp
#define FileController_hpp

#include "rooms/Lobby.hpp"

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
    public:

      ReadCallback(const std::shared_ptr<File::Subscriber>& subscriber)
        : m_subscriber(subscriber)
      {}

      oatpp::v_io_size read(void *buffer, v_buff_size count, oatpp::async::Action& action) override {
        return m_subscriber->readChunk(buffer, count, action);
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

      return _return(OutgoingResponse::createShared(Status::CODE_200, body));
    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif // FileController_hpp
