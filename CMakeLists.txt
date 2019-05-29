cmake_minimum_required(VERSION 3.13)
project(haffman_zip)

set(CMAKE_CXX_STANDARD 14)

add_library(${CMAKE_PROJECT_NAME}_lib SHARED haffman_tree.cpp haffman_tree.h)

add_executable(${CMAKE_PROJECT_NAME}_exe main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}_exe ${CMAKE_PROJECT_NAME}_lib)