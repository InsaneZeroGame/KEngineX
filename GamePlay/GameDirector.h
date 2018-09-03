#pragma once
#include <INoCopy.h>
#include <IModule.h>
#include <AssetManager.h>
#include <IRenderer.h>



//GameDirector coordinates gameplay gamescene and renderer
namespace gameplay
{
    class GameDirector final : public KFramework::IModule,public KFramework::INoCopy
    {
    public:

        static GameDirector& GetGameDirector()
        {
            static GameDirector l_manager;
            return l_manager;
        }

        static GameDirector* GetGameDirectorPtr()
        {
            return &GetGameDirector();
        }

        ~GameDirector();

        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        virtual void Destory() override;

        __forceinline void SetRenderer(Renderer::IRenderer* p_renderer)
        {
            m_renderer = p_renderer;
        }


    private:
        GameDirector();

        assetlib::AssetManager* m_asset_manager = nullptr;

       Renderer::IRenderer* m_renderer = nullptr;

    };//Class GameDirector
}//Namespace gameplay