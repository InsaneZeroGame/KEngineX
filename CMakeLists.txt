cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(KEngineX VERSION 0.1.0 LANGUAGES CXX)

include_directories(Framework)
include_directories(Renderer)
include_directories(GamePlay)


add_subdirectory(Framework)
add_subdirectory(Renderer)
add_subdirectory(GamePlay)
add_subdirectory(WorldEditor)
