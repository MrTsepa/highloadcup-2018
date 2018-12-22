cmake_minimum_required(VERSION 3.7)

project(highloadcup_2018)

set(CMAKE_CXX_STANDARD 14)

find_library(event REQUIRED)
include_directories(${LIBEVENT_INCLUDE_DIR})
include_directories("lib/include")

#add_executable(evhttp_example evhttp_example.cpp)
#add_executable(rapidjson_example rapidjson_example.cpp)
add_executable(main main.cpp)

#target_link_libraries(evhttp_example ${EVENT_LIBRARY})
target_link_libraries(main ${EVENT_LIBRARY})

