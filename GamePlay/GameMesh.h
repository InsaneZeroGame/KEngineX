#pragma once
#include <vector>
#include <array>
#include <unordered_map>

namespace gameplay
{
    struct GameSubMesh
    {
    public:
        GameSubMesh();
        GameSubMesh(std::vector<uint32_t> p_indices);
        ~GameSubMesh();

        struct MeshDescriptor
        {
            //To Support All graphics.
            uint64_t BufferLocation;//D3D12 Buffer View
            uint32_t SizeInBytes;   //D3D12 Buffer View
            uint32_t StrideInBytes; //D3D12 Buffer View

        } m_index_buffer_desc;

        std::vector<uint32_t> m_indices;

        std::array<float, 4> m_diffuse;

        uint32_t m_index_count = 0;

        uint32_t m_texture_id = 0;

        void ReleaseMeshData();
    };


    struct GameMesh
    {
    public:
        GameMesh();

        GameMesh(std::vector<float> p_vertices);

        ~GameMesh();

        struct MeshDescriptor
        {
            //To Support All graphics.
            uint64_t BufferLocation;//D3D12 Buffer View
            uint32_t SizeInBytes;   //D3D12 Buffer View
            uint32_t StrideInBytes; //D3D12 Buffer View

        } m_vertex_buffer_desc;

        std::vector<float> m_vertices;

        std::vector<GameSubMesh> m_sub_meshes;

        std::unordered_multimap<std::string, int32_t> m_texture_map;

        std::vector<std::string> m_texture_names;
        //It's called once data has been uploaded to GPU'S vram.
        void ReleaseMeshData();
        
    };//class GameMesh
}//namespace gameplay