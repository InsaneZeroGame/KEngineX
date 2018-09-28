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

        //It's called once data has been uploaded to GPU'S vram.
        void ReleaseMeshData();

        void AddVertices(const std::vector<float>& p_vertices);

        void AddIndices(const std::vector<uint32_t>& p_indices);

        __forceinline std::vector<float>& GetVertices()
        {
            return m_vertices;
        }

        __forceinline const std::vector<uint32_t>& GetIndices() const
        {
            return m_indices;
        }

        __forceinline void AddIndex(uint32_t p_index)
        {
            m_indices.push_back(p_index);
            m_index_count++;
        };

        __forceinline uint64_t GetIndexCount() const
        {
            return m_index_count;
        }

        __forceinline uint64_t GetIndexOffsetInBuffer() const
        {
            return m_index_offset;
        }

        __forceinline uint64_t GetVertexOffsetInBuffer() const
        {
            return m_vertex_offset;
        }

        __forceinline const void* GetDiffuseMaterial() const
        {
            return m_diffuse.data();
        }

        __forceinline uint32_t GetTextureId() const
        {
            return m_texture_id;
        }

        __forceinline void SetTextureId(uint32_t p_index)
        {
            m_texture_id = p_index;
        }
        void SetVertexOffsetInBuffer(uint64_t p_offset)
        {
            m_vertex_offset = p_offset;
        }

        void SetIndexOffsetInBuffer(uint64_t p_offset)
        {
            m_index_offset = p_offset;
        }


    private:

        

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

       
        
    };//class GameMesh
}//namespace gameplay