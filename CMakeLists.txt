cmake_minimum_required(VERSION 2.8.9)
project(delegate)

include_directories(inc)
include_directories(/usr/local/ACE_TAO-6.5.11/include)
link_directories(/usr/local/ACE_TAO-6.5.11/lib)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")

add_executable(unimanage ${SOURCES})
target_link_libraries(unimanage ACE)

