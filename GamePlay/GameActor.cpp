#include "GameActor.h"
#include <EngineConfig.h>

gameplay::GameRenderActor::GameRenderActor(const std::string& p_name)
    :GameActor(p_name),
    RenderComponent()
{

}


gameplay::GameRenderActor::~GameRenderActor()
{
    
}



void gameplay::RenderComponent::GenerateBoundingBox()
{
    uint32_t fp32_max_raw = 0x7f800000;
    uint32_t fp32_min_raw = 0xff800000;

    auto fp32_max = *((float*)(&fp32_max_raw));
    auto fp32_min = *((float*)(&fp32_min_raw));


    Math::Point l_max = { fp32_min ,fp32_min,fp32_min };
    Math::Point l_min = { fp32_max,fp32_max,fp32_max };

    for (auto mesh : m_meshes)
    {
        assert(!mesh->IsReleased());
        const auto& l_mesh_vertices = mesh->GetVertices();
        for (const auto& vertex : l_mesh_vertices)
        {
            //Max
            if (vertex.m_ui_color_vertex.position[0] > l_max.GetX())
                l_max.SetX(vertex.m_ui_color_vertex.position[0]);
            if (vertex.m_ui_color_vertex.position[1] > l_max.GetY())
                l_max.SetY(vertex.m_ui_color_vertex.position[1]);
            if (vertex.m_ui_color_vertex.position[2] > l_max.GetZ())
                l_max.SetZ(vertex.m_ui_color_vertex.position[2]);
            //Min
            if (vertex.m_ui_color_vertex.position[0] < l_min.GetX())
                l_min.SetX(vertex.m_ui_color_vertex.position[0]);
            if (vertex.m_ui_color_vertex.position[1] < l_min.GetY())
                l_min.SetY(vertex.m_ui_color_vertex.position[1]);
            if (vertex.m_ui_color_vertex.position[2] < l_min.GetZ())
                l_min.SetZ(vertex.m_ui_color_vertex.position[2]);
        }
    }


    m_bounding_box_mesh = std::shared_ptr<GameMesh>(new GameMesh("BoundingBox", Math::BoundingBox(l_max, l_min)));
}




