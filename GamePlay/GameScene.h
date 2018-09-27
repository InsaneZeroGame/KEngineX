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

        std::shared_ptr<GameActor>  dummy_actor;

        
        GameCamera m_main_camera;
        GameCamera m_shadow_camera;

    private:

    };//class GameScene
}//namespace GamePlay