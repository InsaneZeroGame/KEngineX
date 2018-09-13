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

        std::unique_ptr<GameMesh> m_mesh;

        __forceinline void AddMesh(std::unique_ptr<GameMesh> p_mesh)
        {
            m_mesh = std::forward<std::unique_ptr<GameMesh>>(p_mesh);
        }
    };
}
