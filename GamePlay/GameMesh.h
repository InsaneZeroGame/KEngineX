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
        GameMesh(const std::string p_name);

        ~GameMesh();

        std::string m_name;

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

        //It's called once data has been uploaded to GPU'S vram.
        void ReleaseMeshData();

        void AddVertices(const std::vector<float>& p_vertices);
        void AddIndices(const std::vector<uint32_t>& p_indices);
        
    };//class GameMesh
}//namespace gameplay