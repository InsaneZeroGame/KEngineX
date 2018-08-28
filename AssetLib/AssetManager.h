#pragma once
#include <INoCopy.h>
#include <IModule.h>
#include <fbxsdk.h>
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

    private:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        AssetManager();
    };//AssetManager
}//AssetLib