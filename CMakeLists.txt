cmake_minimum_required(VERSION 3.7)

project(highloadcup_2018)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch -O3")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_library(event REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_library(boost 1.68 REQUIRED COMPONENTS container algorithm)

include_directories(${Boost_INCLUDE_DIRS})

include_directories("lib/include")

add_executable(
        main src/main.cpp src/parse_json.hpp src/build_indices.hpp
        src/filter_query_parse.hpp src/group_query_parse.hpp src/types.hpp
        src/utils.hpp
)
target_link_libraries(main event ${BOOST_LIBRARIES})
