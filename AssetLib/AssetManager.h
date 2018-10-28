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

        void LoadSceneContent(const std::string& p_name,gameplay::GamesScene* p_scene);
        
        void LoadMesh(gameplay::GameMesh* p_mesh);

        void LoadMesh(std::shared_ptr<gameplay::GameMesh> p_mesh);
        
        void LoadSceneToRenderer(gameplay::GamesScene*);

    private:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;


    private:

        

        AssetManager();
    };//AssetManager
}//AssetLib