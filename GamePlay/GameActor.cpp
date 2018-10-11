#include "GameActor.h"
#include <EngineConfig.h>

gameplay::GameActor::GameActor(const std::string& p_name)
    :SceneNode(p_name),
    m_meshes({})
{
    m_texture_names.push_back(KEngineConstants::ASSET_DIR + "\\textures\\default_texture1.jpg");
}


gameplay::GameActor::~GameActor()
{
    for (auto mesh : m_meshes) {
        if (mesh)
        {
            delete mesh;
            mesh = nullptr;
        }
    }
}

void gameplay::GameActor::GenerateBoundingBox()
{
    Math::Point l_max = {};
    Math::Point l_min = {};

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




