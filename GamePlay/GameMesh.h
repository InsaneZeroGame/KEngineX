#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <cassert>
namespace gameplay
{
    struct GameMesh
    {
    public:
        GameMesh();

        ~GameMesh();

        std::vector<float> m_vertices;

        std::vector<uint32_t> m_indices;

        uint64_t m_vertex_count = 0;

        uint64_t m_index_count = 0;

        //Offset in global vertex buffer.
        uint64_t m_vertex_offset = 0;
        //Offfset in global index buffer
        uint64_t m_index_offset = 0;

        std::array<float, 4> m_diffuse;

        uint32_t m_texture_id = 0;

        std::unordered_multimap<std::string, int32_t> m_texture_map;

        std::vector<std::string> m_texture_names;
        //It's called once data has been uploaded to GPU'S vram.
        void ReleaseMeshData();

        void AddVertices(const std::vector<float>& p_vertices,uint64_t p_offet);
        void AddIndices(const std::vector<uint32_t>& p_indices,uint64_t p_offset);
        
    };//class GameMesh
}//namespace gameplay