file(GLOB DOT_H ./*.h)
file(GLOB DOT_CPP ./*.cpp)

include_directories(${PROJECT_SOURCE_DIR}/AssetLib)


add_library(Renderer STATIC ${DOT_CPP} ${DOT_H})
target_link_libraries(Renderer Framework d3d12 dxgi d3dcompiler GamePlay AssetLib)