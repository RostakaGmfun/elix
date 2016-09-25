cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

add_library(ecf INTERFACE)
target_include_directories(ecf INTERFACE include/ json/src Catch/single_include)
add_subdirectory(test)
