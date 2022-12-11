cmake_minimum_required(VERSION 3.0.0)
project(youbourse VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

enable_testing()
add_subdirectory(tests)

add_executable(youbourse main.cpp)
