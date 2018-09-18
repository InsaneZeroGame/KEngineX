#include "GameActor.h"

gameplay::GameActor::GameActor(const std::string& p_name)
    :SceneNode(p_name),
    m_mesh(std::unique_ptr<GameMesh>(new GameMesh))
{
}


gameplay::GameActor::~GameActor()
{
}


