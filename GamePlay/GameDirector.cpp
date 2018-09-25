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

    auto res = std::async(std::launch::async,&assetlib::AssetManager::LoadScene,m_asset_manager, "sponza.fbx");
    //m_asset_manager->LoadScene("crytek-sponza.obj");
}

void gameplay::GameDirector::Update()
{
    m_renderer->SetCurrentScene(m_asset_manager->GetScene("sponza.fbx"));
}

void gameplay::GameDirector::Destory()
{
}
