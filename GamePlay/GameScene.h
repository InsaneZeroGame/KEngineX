#pragma once
#include <INoCopy.h>
#include "GameActor.h"
#include "GameCamera.h"

namespace gameplay
{
    class GamesScene final : public KFramework::INoCopy
    {
    public:
        GamesScene();
        ~GamesScene();

        std::shared_ptr<GameActor>  dummy_actor;
        
        GameCamera m_main_camera;

    private:

    };//class GameScene
}//namespace GamePlay