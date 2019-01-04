cmake_minimum_required(VERSION 3.7)

project(highloadcup_2018)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_library(event REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_library(boost 1.68 REQUIRED COMPONENTS multi_index bimap container)

include_directories(${Boost_INCLUDE_DIRS})

include_directories("lib/include")

#add_executable(
#        main1 try1/main.cpp try1/parse_json.hpp try1/types.hpp
#        try1/utils.hpp try1/build_indices.hpp
#        try1/filter_query_parse.hpp try1/merge_sets.hpp
#        try1/trie.hpp try1/group_query_parse.hpp
#)
add_executable(
        main2 try2/main.cpp try2/parse_json.hpp
)
#target_link_libraries(main1 event)
target_link_libraries(main2 event ${BOOST_LIBRARIES})
