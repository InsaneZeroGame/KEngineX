#include "GameActor.h"

gameplay::GameActor::GameActor(const std::string& p_name)
    :SceneNode(p_name),
    m_meshes({})
{
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


