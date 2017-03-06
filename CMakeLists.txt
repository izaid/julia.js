cmake_minimum_required(VERSION 2.6)
project(libj2)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Julia)

find_library(
	V8_LIBRARY
	NAMES v8
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-unused -std=gnu99 -fPIC -Wno-format-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-deprecated-declarations -std=c++14")
set(CMAKE_VERBOSE_MAKEFILE true)

include_directories(
    include
    deps
${Julia_INCLUDE_DIRS}
)



add_subdirectory(tests)

set(libj2_SRC
    src/j2/j2.cpp
)

add_library(j2 SHARED ${libj2_SRC})
target_link_libraries(j2 ${Julia_LIBRARY} v8 v8_libbase v8_libplatform)

install(TARGETS j2 LIBRARY DESTINATION lib)
