cmake_minimum_required(VERSION 2.6)
project(test_libj2)

include_directories(
    ../deps/googletest-release-1.8.0/googletest/include
    ../deps/googletest-release-1.8.0/googlemock/include
)

set(tests_src
    test_j2.cpp
)

add_subdirectory(../deps/googletest-release-1.8.0 "${CMAKE_CURRENT_BINARY_DIR}/deps/googletest-release-1.8.0")

add_executable(test_libj2 ${tests_src})

target_link_libraries(test_libj2 j2 gtest)
