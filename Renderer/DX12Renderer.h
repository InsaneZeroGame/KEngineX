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

        uint32_t m_window_height;

        uint32_t m_window_width;

        DX12GpuDevice* m_device;

        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;

        uint64_t m_current_frameindex;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

        uint32_t m_rtvDescriptorSize;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[DX12RendererConstants::SWAP_CHAIN_COUNT];

    private:

        void InitDevice();

        void InitSwapChain();

    public:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        virtual void Destory() override;

        __forceinline void SetWindow(HWND hWnd,uint64_t height = 1920,uint64_t width = 1080) 
        {
            m_hwnd = hWnd;
        };

    };//DX12Renderer
}//Renderer


