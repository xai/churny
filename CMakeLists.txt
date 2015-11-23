cmake_minimum_required(VERSION 2.8.4)
project(churny)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lgit2")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lgit2")

set(SOURCE_FILES
    src/churny.c
    src/churny.h
    src/list.c
    src/list.h
    src/loc.c
    src/loc.h
    src/utils.c
    src/utils.h
	src/jdime.c
	src/jdime.h)

add_executable(churny ${SOURCE_FILES})
