#include "GameMesh.h"
#include <EngineConfig.h>

namespace gameplay
{

    //Game Mesh contains Vertices.
    GameMesh::GameMesh()
       :m_vertices({}),
        m_indices({}),
        m_texture_names({})
    {
        //Add a default texture.
        //m_texture_map.insert(std::pair<std::string, uint32_t>(KEngineConstants::DEFAULT_TEXTURE_NAME,-1));
        //m_texture_names.push_back(KEngineConstants::DEFAULT_TEXTURE_NAME);
    }

   
    void GameMesh::ReleaseMeshData()
    {

        assert(m_vertices.size() > 0);
        assert(m_indices.size() > 0);

        std::vector<float> l_release_vertices;
        l_release_vertices.clear();
        m_vertices.swap(l_release_vertices);

        std::vector<uint32_t> l_release_indices;
        l_release_indices.clear();
        m_indices.swap(l_release_indices);

    }
    void GameMesh::AddVertices(const std::vector<float>& p_vertices, uint64_t p_offet)
    {
        m_vertices = p_vertices;
        m_vertex_offset = p_offet;

    }
    void GameMesh::AddIndices(const std::vector<uint32_t>& p_indices, uint64_t p_offet)
    {
        m_indices = p_indices;
        m_index_offset;
    }


    GameMesh::~GameMesh()
    {
    }
}