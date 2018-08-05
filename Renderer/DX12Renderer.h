#pragma once
#include <INoCopy.h>
#include <IModule.h>
namespace Renderer {
    class DX12Renderer final: public KFramework::INoCopy,public KFramework::IModule
    {
    public:
        DX12Renderer();
        ~DX12Renderer();

    private:

    public:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        virtual void Destory() override;

    };//DX12Renderer
}//Renderer


