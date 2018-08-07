cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(KEngineX VERSION 0.1.0 LANGUAGES CXX)



add_subdirectory(Framework)
include_directories(Framework)
add_subdirectory(Renderer)
include_directories(Renderer)
add_subdirectory(WorldEditor)
