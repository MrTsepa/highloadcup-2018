cmake_minimum_required(VERSION 3.7)

project(highloadcup_2018)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_library(event REQUIRED)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_library(boost 1.68 REQUIRED COMPONENTS dynamic_bitset bimap)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("lib/include")

add_executable(main main.cpp)
target_link_libraries(main event)
target_link_libraries(main ${Boost_LIBRARIES})

