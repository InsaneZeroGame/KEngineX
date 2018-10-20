#pragma once
#include "SceneNode.h"
#include "GameMesh.h"
#include <unordered_map>
#include <unordered_set>
#include <Math/BoundingBox.h>

namespace gameplay
{
    using TextureId = uint32_t;

    class RenderComponent
    {
    public:
        RenderComponent():
            m_meshes({})
        {
            m_texture_names.push_back(KEngineConstants::ASSET_DIR + "\\textures\\default_texture1.jpg");
        };
        virtual ~RenderComponent()
        {
            for (auto mesh : m_meshes) {
                if (mesh)
                {
                    delete mesh;
                    mesh = nullptr;
                }
            }
        };

        __forceinline void AddMesh(GameMesh* p_mesh)
        {
            m_meshes.push_back(p_mesh);
        }

        void GenerateBoundingBox();

        std::vector<GameMesh*> m_meshes;

        std::unordered_multimap<std::string, int32_t> m_texture_map;

        std::vector<std::string> m_texture_names;

        std::shared_ptr<GameMesh> m_bounding_box_mesh;
    };

    class GameActor : public SceneNode
    {
    public:
        GameActor(const std::string& p_name)
            :SceneNode(p_name)
        {
        
        };
        virtual ~GameActor()
        {

        };
    };

    



    class GameRenderActor : public GameActor, public RenderComponent
    {
    public:
        GameRenderActor(const std::string& p_name);

        virtual ~GameRenderActor();

    };
}
