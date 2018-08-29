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
        m_index_count(p_indices.size())
    {
    
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