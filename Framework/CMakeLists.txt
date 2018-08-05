file(GLOB DOT_H ./*.h)
file(GLOB DOT_CPP ./*.cpp)




add_library(Framework STATIC ${DOT_CPP} ${DOT_H})
