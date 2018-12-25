FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y cmake wget gcc g++ unzip

RUN wget https://github.com/libevent/libevent/releases/download/release-2.1.8-stable/libevent-2.1.8-stable.tar.gz
RUN tar xzf libevent-2.1.8-stable.tar.gz
WORKDIR /libevent-2.1.8-stable
RUN ./configure --prefix=/usr/local
RUN make
RUN make install

EXPOSE 80

ENV DATA_PATH=/tmp/data/data.zip PORT=80 HOST=0.0.0.0 START_SERVER=1 \
    OPTIONS_PATH=/tmp/data/options.txt

WORKDIR /app
ADD lib/ ./lib/
ADD main.cpp build_indices.hpp merge_sets.hpp parse_json.hpp parse_query.hpp \
    types.hpp utils.hpp ./
ADD CMakeLists.txt .

WORKDIR /app/build

RUN cmake ..
RUN cmake --build .

CMD ./main