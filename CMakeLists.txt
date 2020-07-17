cmake_minimum_required(VERSION 2.8.12)
project(Benchmarks)

add_definitions("-std=c++17")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(benchmarks benchmarks.cpp)
target_link_libraries(benchmarks ${CONAN_LIBS})
