#include "GameDirector.h"
#include <future>

gameplay::GameDirector::GameDirector()
{
    Init();
}

gameplay::GameDirector::~GameDirector()
{
}

void gameplay::GameDirector::Init()
{
    m_asset_manager = assetlib::AssetManager::GetAssertManagerPtr();

    std::string scene_name = "GameScene.fbx";

    //auto res = std::async(std::launch::async,&assetlib::AssetManager::LoadScene,m_asset_manager, scene_name);
    assetlib::AssetManager::GetAssertManager().LoadScene(scene_name);
    assetlib::AssetManager::GetAssertManager().LoadSceneToRenderer(scene_name);
    //m_asset_manager->LoadScene("crytek-sponza.obj");
}

void gameplay::GameDirector::Update()
{
    std::string scene_name = "GameScene.fbx";

    m_renderer->SetCurrentScene(m_asset_manager->GetScene(scene_name));
}

void gameplay::GameDirector::Destory()
{
}
