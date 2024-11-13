cmake_minimum_required(VERSION 3.10)
project(stream_analyser VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -w -fdiagnostics-color=always -pthread")

add_executable(stream_analyser "main.cpp" "stream_analyser.cpp")