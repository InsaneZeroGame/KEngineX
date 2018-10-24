#pragma once
#include <INoCopy.h>
#include <IModule.h>
#include <fbxsdk.h>
#include <string>
#include "DX12TransferManager.h"
#include <unordered_map>
#include <GameScene.h>
namespace assetlib
{
    class AssetManager final: public KFramework::IModule,public KFramework::INoCopy
    {
    public:

        static AssetManager& GetAssertManager()
        {
            static AssetManager l_manager;
            return l_manager;
        }

        static AssetManager* GetAssertManagerPtr()
        {
            return &GetAssertManager();
        }

        
        
        ~AssetManager();

    public:
        
        virtual void Destory() override;

        void LoadScene(const std::string& p_name);

        void LoadMesh(gameplay::GameMesh* p_mesh);

        void LoadSceneToRenderer(const std::string& p_name);

        void LoadMesh(std::shared_ptr<gameplay::GameMesh> p_mesh);

        __forceinline std::shared_ptr<gameplay::GamesScene> GetScene(const std::string& p_name)
        {
            return m_scenes[p_name];
        }

    private:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

    private:

        using SceneMap = std::unordered_map < std::string, std::shared_ptr<gameplay::GamesScene>>;

         SceneMap m_scenes;

        AssetManager();
    };//AssetManager
}//AssetLib