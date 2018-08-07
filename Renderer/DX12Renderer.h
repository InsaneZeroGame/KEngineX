#pragma once
#include <INoCopy.h>
#include <IModule.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#include "DX12GpuDevice.h"

namespace Renderer {
    class DX12Renderer final: public KFramework::INoCopy,public KFramework::IModule
    {
    public:
        DX12Renderer();
        ~DX12Renderer();
    private:

        HWND m_hwnd;

        DX12GpuDevice* m_device;
    private:

        void InitDevice();

    public:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        virtual void Destory() override;

        __forceinline void SetWindow(HWND hWnd) 
        {
            m_hwnd = hWnd;
        };

    };//DX12Renderer
}//Renderer


