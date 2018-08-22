#pragma once
#include "SceneNode.h"
#include "GameMesh.h"

namespace gameplay
{
    class GameActor : public SceneNode
    {
    public:
        GameActor(const std::string& p_name);

        virtual ~GameActor();

        void AddMaterial(std::shared_ptr<GameMeterial> p_mesh);
        
        std::vector<std::shared_ptr<GameMeterial>> m_meterial;
    };
}
