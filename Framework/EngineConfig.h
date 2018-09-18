#pragma once


namespace KEngineConstants
{
    const unsigned int WINDOW_X = 0;
    const unsigned int WINDOW_Y = 0;
    const unsigned int WINDOW_WIDTH = 1920;
    const unsigned int WINDOW_HEIGHT = 1080;
    const unsigned int VERTEX_INDEX_BUFFER_SIZE_IN_BYTE = 100 * 1024 * 1024; // 100Mb
    const unsigned int TEXTURE_UPLOAD_BUFFER_SIZE_IN_BYTE = 100 * 1024 * 1024; // 10Mb
    const std::string MODEL_ASSET_DIR = "C:\\Users\\angl\\Documents\\GitHub\\KEngineX\\Assets\\Models\\";
    const std::string ASSET_DIR = "C:\\Users\\angl\\Documents\\GitHub\\KEngineX\\Assets\\";
    const std::string DEFAULT_TEXTURE_NAME = "KEngineDefaultTexture";
    const std::wstring SHADER_ASSET_DIR = L"C:\\Users\\angl\\Documents\\GitHub\\KEngineX\\Assets\\Shaders\\";
}




namespace Renderer 
{
    struct Vertex
    {
        float position[3];
        float color[4];
        float texture_coord[2];
    };
}