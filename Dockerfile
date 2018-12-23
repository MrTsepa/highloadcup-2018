FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y cmake wget
RUN wget https://github.com/libevent/libevent/releases/download/release-2.1.8-stable/libevent-2.1.8-stable.tar.gz
RUN tar xzf libevent-2.1.8-stable.tar.gz
WORKDIR /libevent-2.1.8-stable
RUN ./configure --prefix=/usr/local
RUN make
RUN make install
WORKDIR /app

RUN apt-get install -y gcc g++ unzip

ADD lib/ ./lib/
ADD main.cpp .
ADD CMakeLists.txt .

EXPOSE 80

ENV DATA_PATH=/tmp/data/data.zip
ENV PORT=80
ENV HOST=0.0.0.0

WORKDIR /app/build

RUN cmake ..
RUN cmake --build .

CMD ./main