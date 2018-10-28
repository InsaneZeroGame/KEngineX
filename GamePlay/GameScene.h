#pragma once
#include <INoCopy.h>
#include "GameActor.h"
#include "GameUIActor.h"
#include "GameCamera.h"
#include "SceneNode.h"

namespace gameplay
{
    class GamesScene final : public SceneNode
    {
    public:
        GamesScene(const std::string& p_name);

        ~GamesScene();

        __forceinline GameRenderActor* GetActor(const std::string& p_name)
        {
            for (auto actor : m_actors)
            {
                if (actor->GetName() == p_name)
                {
                    return actor;
                }
            }
            return nullptr;
        }

        void LoadSceneContent(const std::string& p_name);

        __forceinline void AddActor(std::string p_name)
        {
            m_actors.push_back(new GameRenderActor(p_name));
        }

        __forceinline void AddActor(GameRenderActor * p_actor)
        {
            m_actors.push_back(p_actor);
        }

        __forceinline std::vector<GameRenderActor*>& GetActors()
        {
            return m_actors;
        }

        __forceinline GameCamera& GetMainCamera()
        {
            return m_main_camera;
        }

        __forceinline GameCamera& GetShadowCamera()
        {
            return m_shadow_camera;
        }

    private:
        
        std::vector<GameRenderActor*> m_actors;
        
        GameCamera m_main_camera;
        
        GameCamera m_shadow_camera;
    };//class GameScene
}//namespace GamePlay