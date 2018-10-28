#pragma once
#include <INoCopy.h>
#include <IModule.h>
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

        __forceinline std::shared_ptr<gameplay::GamesScene> GetScene(const std::string& p_name)
        {
            return m_scenes[p_name];
        }

    private:
        GameDirector();

       Renderer::IRenderer* m_renderer = nullptr;

       using SceneMap = std::unordered_map < std::string, std::shared_ptr<gameplay::GamesScene>>;

       SceneMap m_scenes;

    };//Class GameDirector
}//Namespace gameplay