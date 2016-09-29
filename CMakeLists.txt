cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(elix INTERFACE)
target_include_directories(elix INTERFACE include/ json/src Catch/single_include)
add_subdirectory(test)
