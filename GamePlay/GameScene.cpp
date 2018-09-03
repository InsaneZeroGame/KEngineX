#include "GameScene.h"

gameplay::GamesScene::GamesScene(const std::string& p_name) :
    SceneNode(p_name),
    dummy_actor(std::make_shared<GameActor>("DummyActorLiu")),
    m_main_camera("MainCamera")
{
}

gameplay::GamesScene::~GamesScene()
{
}
