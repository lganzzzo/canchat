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

#include "controller/FileController.hpp"
#include "controller/RoomsController.hpp"
#include "controller/StaticController.hpp"

#include "./AppComponent.hpp"

#include "oatpp/network/server/Server.hpp"

#include <iostream>

void run(const oatpp::base::CommandLineArguments& args) {

  /* Register Components in scope of run() method */
  AppComponent components(args);

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create RoomsController and add all of its endpoints to router */
  auto roomsController = std::make_shared<RoomsController>();
  roomsController->addEndpointsToRouter(router);

  auto staticController = std::make_shared<StaticController>();
  staticController->addEndpointsToRouter(router);

  auto fileController = std::make_shared<FileController>();
  fileController->addEndpointsToRouter(router);

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, connectionHandler, "http");

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::server::Server server(connectionProvider, connectionHandler);

  std::thread serverThread([&server]{
    server.run();
  });

  std::thread pingThread([]{
    OATPP_COMPONENT(std::shared_ptr<Lobby>, lobby);
    lobby->runPingLoop(std::chrono::seconds(30));
  });

  OATPP_COMPONENT(ConfigDto::ObjectWrapper, appConfig);

  if(appConfig->useTLS) {
    OATPP_LOGI("canchat", "clients are expected to connect at https://%s:%d/", appConfig->host->getData(), appConfig->port->getValue());
  } else {
    OATPP_LOGI("canchat", "clients are expected to connect at http://%s:%d/", appConfig->host->getData(), appConfig->port->getValue());
  }

  serverThread.join();

}

int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();

  run(oatpp::base::CommandLineArguments(argc, argv));

  oatpp::base::Environment::destroy();

  return 0;
}