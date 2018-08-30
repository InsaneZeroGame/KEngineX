file(GLOB DOT_H ./*.h ./Math/*.h ./Math/*.inl)
file(GLOB DOT_CPP ./*.cpp ./Math/*.cpp)




add_library(Framework STATIC ${DOT_CPP} ${DOT_H})
