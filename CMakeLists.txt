cmake_minimum_required(VERSION 3.21)

project(G2OWS VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(G2OWS_MODULE G2OWS)

add_library(${G2OWS_MODULE} SHARED)

add_subdirectory(dependencies)
add_subdirectory(source)