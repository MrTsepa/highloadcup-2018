cmake_minimum_required(VERSION 3.7)

project(highloadcup_2018)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_library(event REQUIRED)

include_directories("lib/include")

add_executable(main main.cpp parse_json.hpp types.hpp utils.hpp build_indices.hpp parse_query.hpp merge_sets.hpp trie.hpp)
target_link_libraries(main event)
