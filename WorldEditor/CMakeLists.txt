file(GLOB DOT_H ./*.h)
file(GLOB DOT_CPP ./*.cpp)



add_executable(WorldEditor WIN32 ${DOT_H} ${DOT_CPP})
target_link_libraries(WorldEditor Renderer)