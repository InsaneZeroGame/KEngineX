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

        //std::shared_ptr<GameRenderActor>  dummy_actor;

        std::vector<GameRenderActor*> m_actors;
        
        GameCamera m_main_camera;
        GameCamera m_shadow_camera;

        void AddActor(std::string p_name);

    private:

    };//class GameScene
}//namespace GamePlay