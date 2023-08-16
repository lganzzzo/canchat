cmake_minimum_required(VERSION 3.20)

set(project_name canchat) ## rename your project here

project(${project_name})

set(CMAKE_CXX_STANDARD 11)

include_directories(src)

add_library(${project_name}-lib
        src/AppComponent.hpp
        src/controller/FileController.hpp
        src/controller/RoomsController.hpp
        src/controller/StaticController.hpp
        src/controller/StatisticsController.hpp
        src/rooms/File.cpp
        src/rooms/File.hpp
        src/rooms/Peer.cpp
        src/rooms/Peer.hpp
        src/rooms/Room.cpp
        src/rooms/Room.hpp
        src/rooms/Lobby.cpp
        src/rooms/Lobby.hpp
        src/utils/Nickname.cpp
        src/utils/Nickname.hpp
        src/utils/Statistics.cpp
        src/utils/Statistics.hpp
        src/dto/DTOs.hpp
        src/dto/Config.hpp
)

## link libs

find_package(oatpp              1.3.0 REQUIRED)
find_package(oatpp-websocket    1.3.0 REQUIRED)
find_package(oatpp-openssl      1.3.0 REQUIRED)

find_package(OpenSSL 1.1 REQUIRED)

target_link_libraries(${project_name}-lib

        # Oat++ libraries
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
        PUBLIC oatpp::oatpp-websocket
        PUBLIC oatpp::oatpp-openssl

        # OpenSSL libraries
        PUBLIC OpenSSL::SSL
        PUBLIC OpenSSL::Crypto

)

#################################################################
## define certificates path

add_definitions(
        -DCERT_PEM_PATH="${CMAKE_CURRENT_LIST_DIR}/../cert/test_key.pem"
        -DCERT_CRT_PATH="${CMAKE_CURRENT_LIST_DIR}/../cert/test_cert.crt"
        -DFRONT_PATH="${CMAKE_CURRENT_LIST_DIR}/../front"
)

#################################################################
## add executables

add_executable(${project_name}-exe
        src/App.cpp
)
target_link_libraries(${project_name}-exe ${project_name}-lib)
add_dependencies(${project_name}-exe ${project_name}-lib)

add_executable(${project_name}-test
        test/tests.cpp
        test/WSTest.cpp
        test/WSTest.hpp
)
target_link_libraries(${project_name}-test ${project_name}-lib)
add_dependencies(${project_name}-test ${project_name}-lib)

set_target_properties(${project_name}-lib ${project_name}-exe ${project_name}-test PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        LINKER_LANGUAGE CXX
)

enable_testing()
add_test(tests ${project_name}-test)
