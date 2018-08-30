#include "GameScene.h"

gameplay::GamesScene::GamesScene() :
    dummy_actor(std::make_shared<GameActor>("DummyActorLiu")),
    m_main_camera("MainCamera")
{
}

gameplay::GamesScene::~GamesScene()
{
}
