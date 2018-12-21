cmake_minimum_required(VERSION 3.13)

project(highloadcup_2018)

set(CMAKE_CXX_STANDARD 14)



find_library(event REQUIRED)
include_directories(${LIBEVENT_INCLUDE_DIR})
include_directories("lib/include")
find_library(zlib REQUIRED)

add_executable(evhttp_example evhttp_example.cpp)
add_executable(rapidjson_example rapidjson_example.cpp)
add_executable(main main.cpp zstr.hpp strict_fstream.hpp)

target_link_libraries(evhttp_example ${EVENT_LIBRARY})
target_link_libraries(main ${Zlib_LIBRARIES})
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS iostreams)

include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(main ${Boost_LIBRARIES})
