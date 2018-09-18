#pragma once
#include "SceneNode.h"
#include "GameMesh.h"
#include <unordered_map>

namespace gameplay
{
    using TextureId = uint32_t;

    class GameActor : public SceneNode
    {
    public:
        GameActor(const std::string& p_name);

        virtual ~GameActor();

        std::vector<GameMesh*> m_meshes;

        __forceinline void AddMesh(GameMesh* p_mesh)
        {
            m_meshes.push_back(p_mesh);
        }


    };
}
