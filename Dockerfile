FROM lganzzzo/ubuntu-cmake-libressl:latest

WORKDIR /chat

ADD cert cert
ADD front front
ADD server server
ADD utility utility

WORKDIR /chat/utility

RUN /bin/bash ./install-oatpp-modules.sh Release

WORKDIR /chat/server/build

RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make

ENTRYPOINT ["./canchat-exe"]
