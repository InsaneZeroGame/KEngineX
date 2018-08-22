#include "GameScene.h"

gameplay::GamesScene::GamesScene() :
    dummy_actor(std::make_shared<GameActor>("DummyActorLiu"))
{
}

gameplay::GamesScene::~GamesScene()
{
}
