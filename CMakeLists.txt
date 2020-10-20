cmake_minimum_required(VERSION 2.8.9)
project(delegate)

#Bring the headers, such as Student.h into the project
include_directories(inc)

#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cc")

add_executable(unimanage ${SOURCES})

