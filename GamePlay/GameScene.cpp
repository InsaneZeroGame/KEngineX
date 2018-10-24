#include "GameScene.h"

gameplay::GamesScene::GamesScene(const std::string& p_name) :
    SceneNode(p_name),
    //dummy_actor(std::make_shared<GameRenderActor>("DummyActorLiu")),
    m_actors({}),
    m_main_camera("MainCamera"),
    m_shadow_camera("ShadowCamera")
{

}

void gameplay::GamesScene::AddActor(std::string p_name)
{
    m_actors.push_back(new GameRenderActor(p_name));
}

gameplay::GamesScene::~GamesScene()
{
    for (auto actor : m_actors)
    {
        if (actor)
        {
            delete actor;
            actor = nullptr;
        }
    }
}
