#include "GameDirector.h"

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
    m_asset_manager->LoadScene("simple_scene.obj");
}

void gameplay::GameDirector::Update()
{
    m_renderer->SetCurrentScene(m_asset_manager->GetScene("simple_scene.obj"));
}

void gameplay::GameDirector::Destory()
{
}
