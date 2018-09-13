#include "GameMesh.h"
#include <EngineConfig.h>

namespace gameplay
{

    //Game Mesh contains Vertices.
    GameMesh::GameMesh()
       :m_vertices({}),
        m_sub_meshes({}),
        m_texture_names({})
    {
        //Add a default texture.
        //m_texture_map.insert(std::pair<std::string, uint32_t>(KEngineConstants::DEFAULT_TEXTURE_NAME,-1));
        //m_texture_names.push_back(KEngineConstants::DEFAULT_TEXTURE_NAME);
    }

    GameMesh::GameMesh(std::vector<float> p_vertices)
        :m_vertices(p_vertices),
        m_sub_meshes({}),
        m_texture_names({})
    {
        //Add a default texture.
        //m_texture_map.insert(std::pair<std::string, uint32_t>(KEngineConstants::DEFAULT_TEXTURE_NAME, -1));
        //m_texture_names.push_back(KEngineConstants::DEFAULT_TEXTURE_NAME);
    }

    void GameMesh::ReleaseMeshData()
    {
        std::vector<float> l_release_vertices;
        l_release_vertices.clear();
        m_vertices.swap(l_release_vertices);
        for (auto& submesh : m_sub_meshes)
        {
            submesh.ReleaseMeshData();
        }
    }


    GameMesh::~GameMesh()
    {
    }
    
   
    GameSubMesh::GameSubMesh():
        m_indices({})
    {
    }
    GameSubMesh::GameSubMesh(std::vector<uint32_t> p_indices):
        m_indices(p_indices),
        m_index_count(static_cast<uint32_t>(p_indices.size()))
    {

    }
    GameSubMesh::~GameSubMesh()
    {
    }
    void GameSubMesh::ReleaseMeshData()
    {
        std::vector<uint32_t> l_release_indices;
        l_release_indices.clear();
        m_indices.swap(l_release_indices);

    }
}