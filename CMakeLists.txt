cmake_minimum_required(VERSION 3.13)
project(huffman_zip)

set(CMAKE_CXX_STANDARD 17)

add_library(${CMAKE_PROJECT_NAME}_lib SHARED huffman_zipper.cpp huffman_zipper.h)

add_executable(${CMAKE_PROJECT_NAME}_exe main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}_exe ${CMAKE_PROJECT_NAME}_lib)