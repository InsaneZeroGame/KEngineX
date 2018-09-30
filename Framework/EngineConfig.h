#pragma once
#include <array>
namespace KEngineConstants
{
    const std::array<float, 3> MESH_DEFAULT_COLOR = { 0.0f, 1.0f, 0.0f };   
    const unsigned int WINDOW_X = 0;
    const unsigned int WINDOW_Y = 0;
    const unsigned int WINDOW_WIDTH = 1920;
    const unsigned int WINDOW_HEIGHT = 1080;
    //sconst unsigned int FLOAT_COUNT_PER_VERTEX = 9;
    const unsigned int VERTEX_INDEX_BUFFER_SIZE_IN_BYTE = 100 * 1024 * 1024; // 100Mb
    const unsigned int TEXTURE_UPLOAD_BUFFER_SIZE_IN_BYTE = 100 * 1024 * 1024; // 10Mb
    const std::string ASSET_DIR = "C:/Users/angl/Documents/GitHub/KEngineX/Assets\\";
    const std::string MODEL_ASSET_DIR = ASSET_DIR +  "Models\\";
    const std::string DEFAULT_TEXTURE_NAME = "KEngineDefaultTexture";
    const std::wstring SHADER_ASSET_DIR = L"C:/Users/angl/Documents/GitHub/KEngineX/Assets/Shaders/";
}




namespace Renderer 
{
   


    union Vertex
    {
        struct NormalVertex
        {
            std::array<float,3> position;
            std::array<float,4> normal;
            std::array<float,2> texture_coord;
        }m_nomral_vertex;

        struct UITextureVertex
        {
            std::array<float,3> position;
            std::array<float,2> texture_coord;
            std::array<float,4> padding;
        }m_ui_texture_vertex;

        struct UIColorVertex
        {
            std::array<float,3> position;
            std::array<float,4> color;
            std::array<float,2> padding;
        }m_ui_color_vertex;

        std::array<float, 9> m_raw = {};

        Vertex(std::array<float,9> p_vertex):m_raw(p_vertex)
        {
           
        }

        Vertex()
        {
            
        }

    };


}