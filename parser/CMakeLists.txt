cmake_minimum_required(VERSION 2.8.9)
project(parser)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(jparser src/jsonparser.yy ${CMAKE_CURRENT_BINARY_DIR}/jsonparser.cpp COMPILE_FLAGS -d)
FLEX_TARGET(jscanner src/jsonlexer.ll  ${CMAKE_CURRENT_BINARY_DIR}/jsonlexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(jscanner jparser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(inc)
include_directories(/usr/local/ACE_TAO-6.5.11/include)
link_directories(/usr/local/ACE_TAO-6.5.11/lib)

file(GLOB SOURCES "src/*.cc")

add_library(json-parser STATIC ${SOURCES}
                        "${CMAKE_CURRENT_BINARY_DIR}/jsonparser.cpp"
                        "${CMAKE_CURRENT_BINARY_DIR}/jsonlexer.cpp")

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")


