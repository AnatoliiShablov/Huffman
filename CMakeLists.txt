cmake_minimum_required(VERSION 3.13)
project(huffman_zip)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_library(${CMAKE_PROJECT_NAME}_lib SHARED huffman_zipper.cpp huffman_zipper.h)

add_library(${CMAKE_PROJECT_NAME}_lib_file_api SHARED huffman_file_api.h huffman_file_api.cpp)

add_executable(${CMAKE_PROJECT_NAME}_exe main.cpp huffman_file_api.h)
target_link_libraries(${CMAKE_PROJECT_NAME}_exe ${CMAKE_PROJECT_NAME}_lib)
target_link_libraries(${CMAKE_PROJECT_NAME}_exe ${CMAKE_PROJECT_NAME}_lib_file_api)

add_executable(${CMAKE_PROJECT_NAME}_test test.cpp huffman_file_api.h)
target_link_libraries(${CMAKE_PROJECT_NAME}_test ${CMAKE_PROJECT_NAME}_lib)
target_link_libraries(${CMAKE_PROJECT_NAME}_test ${CMAKE_PROJECT_NAME}_lib_file_api)
