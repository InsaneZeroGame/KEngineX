#include "GameScene.h"
#include <AssetManager.h>

gameplay::GamesScene::GamesScene(const std::string& p_name) :
    SceneNode(p_name),
    //dummy_actor(std::make_shared<GameRenderActor>("DummyActorLiu")),
    m_actors({}),
    m_main_camera("MainCamera"),
    m_shadow_camera("ShadowCamera")
{

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

void gameplay::GamesScene::LoadSceneContent(const std::string& p_name)
{
    assetlib::AssetManager::GetAssertManager().LoadSceneContent(p_name, this);

}
