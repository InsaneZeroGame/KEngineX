#pragma once
#include "SceneNode.h"
#include "GameMesh.h"
#include <unordered_map>
#include <unordered_set>
#include <Math/BoundingBox.h>

namespace gameplay
{
    using TextureId = uint32_t;

    class GameActor : public SceneNode
    {
    public:
        GameActor(const std::string& p_name);

        virtual ~GameActor();

        std::vector<GameMesh*> m_meshes;

        std::unordered_multimap<std::string, int32_t> m_texture_map;

        std::vector<std::string> m_texture_names;

        std::shared_ptr<GameMesh> m_bounding_box_mesh;

        __forceinline void AddMesh(GameMesh* p_mesh)
        {
            m_meshes.push_back(p_mesh);
        }

        void GenerateBoundingBox();


    };
}
