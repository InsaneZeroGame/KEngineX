#include "GameActor.h"

gameplay::GameActor::GameActor(const std::string& p_name)
    :SceneNode(p_name),
    m_meterial({})
{

}


void gameplay::GameActor::AddMaterial(std::shared_ptr<GameMeterial> p_mesh)
{
     m_meterial.push_back(p_mesh);
}


gameplay::GameActor::~GameActor()
{

}
