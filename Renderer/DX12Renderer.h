#pragma once
#include <IRenderer.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#include <array>
#include "DX12GpuDevice.h"
#include "DX12ComandBuffer.h"
#include "DX12TransferManager.h"

namespace Renderer {
    class DX12Renderer final: public IRenderer
    {
    public:
        DX12Renderer();
        ~DX12Renderer();
    private:

        DX12GpuDevice* m_device;

        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;

        HANDLE m_fenceEvent;

        uint8_t m_current_frameindex;

        uint64_t m_fence_value[DX12RendererConstants::SWAP_CHAIN_COUNT];

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

        uint32_t m_rtvDescriptorSize;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[DX12RendererConstants::SWAP_CHAIN_COUNT];

        std::array<DX12RenderCommndBuffer*, DX12RendererConstants::SWAP_CHAIN_COUNT> m_render_cmd;

        std::array<ID3D12Fence*, DX12RendererConstants::SWAP_CHAIN_COUNT> m_fences;
   
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

        //Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
        
        CD3DX12_VIEWPORT m_viewport;

        CD3DX12_RECT m_scissorRect;

        //D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        std::shared_ptr<gameplay::GamesScene> m_scene;
    private:

        void InitDevice();

        void InitSwapChain();

        void InitFences();

        void InitGraphicsPipelines();

        void WaitForPreviousFrame();

        void RecordGraphicsCmd();

        void InitCmdBuffers();

    public:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        virtual void Destory() override;

        virtual void SetWindow(HWND hWnd, uint32_t height, uint32_t width) override;

        virtual void LoadScene(std::shared_ptr<gameplay::GamesScene>) override;

    };//DX12Renderer
}//Renderer


