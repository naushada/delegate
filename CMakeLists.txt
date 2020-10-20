cmake_minimum_required(VERSION 2.8.9)
project(delegate)

include_directories(inc)
include_directories(/usr/local/ACE_TAO-6.5.11/include)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")

set(PROJECT_LINK_LIBS libACE.so)
link_directories(/usr/local/ACE_TAO-6.5.11/lib)

add_executable(unimanage ${SOURCES})

