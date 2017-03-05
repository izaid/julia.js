cmake_minimum_required(VERSION 2.6)
project(libj2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-unused -std=gnu99 -fPIC -Wno-format-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-deprecated-declarations -std=c++14")
set(CMAKE_VERBOSE_MAKEFILE true)

include_directories(
    include
    deps
)

add_subdirectory(tests)

set(libj2_SRC
    src/j2/j2.cpp
)

add_library(j2 SHARED ${libj2_SRC})
