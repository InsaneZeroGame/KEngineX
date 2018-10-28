#include "GameDirector.h"
#include <future>
std::string dummy_scene_name = "GameScene.fbx";


gameplay::GameDirector::GameDirector()
{
    Init();
}

gameplay::GameDirector::~GameDirector()
{
}

void gameplay::GameDirector::Init()
{
    auto l_scene = new gameplay::GamesScene(dummy_scene_name);
    
    l_scene->LoadSceneContent(dummy_scene_name);
    
    m_scenes[dummy_scene_name] = std::shared_ptr<gameplay::GamesScene>(l_scene);


}

void gameplay::GameDirector::Update()
{
    m_renderer->SetCurrentScene(m_scenes[dummy_scene_name]);
}

void gameplay::GameDirector::Destory()
{
}
