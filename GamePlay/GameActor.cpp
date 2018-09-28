#include "GameActor.h"

gameplay::GameActor::GameActor(const std::string& p_name)
    :SceneNode(p_name),
    m_meshes({})
{
    m_texture_names.push_back("C:/Users/angl/Documents/GitHub/KEngineX/Assets/textures/default_texture1.jpg");
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


