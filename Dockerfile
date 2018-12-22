FROM gcc:latest as build

RUN apt-get update
RUN apt-get install -y cmake libevent-dev

WORKDIR /app

ADD main.cpp .
ADD CMakeLists.txt .
ADD lib/ ./lib/

WORKDIR /app/build

RUN cmake ..
RUN cmake --build .

FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y unzip

WORKDIR /app

COPY --from=build /app/build/main .

EXPOSE 80

ENV DATA_PATH=/tmp/data/data.zip

CMD ./main