#include "GameMesh.h"
#include <EngineConfig.h>

namespace gameplay
{

    //Game Mesh contains Vertices.
    GameMesh::GameMesh(const std::string p_name)
       :m_vertices({}),
        m_name(p_name),
        m_indices({})
    {
        
    }

    GameMesh::GameMesh(const std::string p_name,const Math::Rect& p_rect, const std::array<float, 3>& p_color)
        :m_name(p_name)
    {
        AddVertices
        ({
            //Top left
            p_rect.m_pos[0].GetX(),p_rect.m_pos[0].GetY(),p_rect.m_pos[0].GetZ(),p_color[0],p_color[1],p_color[2],1.0f,0.0f,0.0f,
            p_rect.m_pos[1].GetX(),p_rect.m_pos[1].GetY(),p_rect.m_pos[1].GetZ(),p_color[0],p_color[1],p_color[2],1.0f,0.0f,1.0f,
            p_rect.m_pos[2].GetX(),p_rect.m_pos[2].GetY(),p_rect.m_pos[2].GetZ(),p_color[0],p_color[1],p_color[2],1.0f,1.0f,1.0f,
            p_rect.m_pos[3].GetX(),p_rect.m_pos[3].GetY(),p_rect.m_pos[3].GetZ(),p_color[0],p_color[1],p_color[2],1.0f,1.0f,0.0f,

         });
        AddIndices({0,1,2,0,2,3});
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
    void GameMesh::AddVertices(const std::vector<float>& p_vertices)
    {
        m_vertices = p_vertices;
        m_vertex_count = p_vertices.size() / KEngineConstants::FLOAT_COUNT_PER_VERTEX;

    }
    void GameMesh::AddIndices(const std::vector<uint32_t>& p_indices)
    {
        m_indices = p_indices;
        m_index_count = p_indices.size();
    }

    void GameMesh::AddVertices(std::vector<float>&& p_vertices)
    {
        m_vertices = p_vertices;
        m_vertex_count = p_vertices.size() / KEngineConstants::FLOAT_COUNT_PER_VERTEX;

    }
    void GameMesh::AddIndices(std::vector<uint32_t>&& p_indices)
    {
        m_indices = p_indices;
        m_index_count = p_indices.size();
    }


    GameMesh::~GameMesh()
    {
    }
}