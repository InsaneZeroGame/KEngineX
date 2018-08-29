#include "GameMesh.h"


namespace gameplay
{

    //Game Mesh contains Vertices.
    GameMesh::GameMesh()
       :m_vertices({}),
        m_indices({})
    {
    }

    GameMesh::GameMesh(std::vector<float> p_vertices, std::vector<uint32_t> p_indices)
        :m_vertices(p_vertices),
        m_indices(p_indices),
        m_index_count(static_cast<uint32_t>(p_indices.size()))
    {
    
    }

    void GameMesh::ReleaseMeshData()
    {
        std::vector<float> l_release_vertices;
        std::vector<uint32_t> l_release_indices;
        l_release_vertices.clear();
        l_release_indices.clear();
        m_vertices.swap(l_release_vertices);
        m_indices.swap(l_release_indices);
    }


    GameMesh::~GameMesh()
    {
    }
    
    //Game Materials
    GameMeterial::GameMeterial():
        m_meshes({})
    {
    }
    GameMeterial::~GameMeterial()
    {
    }
}