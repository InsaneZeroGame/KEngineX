#pragma once
#include <INoCopy.h>
#include "GameActor.h"

namespace gameplay
{
    class GamesScene final : public KFramework::INoCopy
    {
    public:
        GamesScene();
        ~GamesScene();

        std::shared_ptr<GameActor>  dummy_actor;

    private:

    };//class GameScene
}//namespace GamePlay