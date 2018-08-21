file(GLOB DOT_H ./*.h)
file(GLOB DOT_CPP ./*.cpp)



add_library(GamePlay STATIC ${DOT_CPP} ${DOT_H})
target_link_libraries(GamePlay Framework)