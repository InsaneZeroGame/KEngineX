#pragma once
#include <IRenderer.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include "DX12DepthBuffer.h"
#include <windows.h>
#include <array>
#include "DX12GpuDevice.h"
#include "DX12ComandBuffer.h"
#include "DX12TransferManager.h"
#include <GameCamera.h>
#include <unordered_map>
#include "DX12Texture.h"

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

        std::array<std::unique_ptr<DX12RenderCommndBuffer>, DX12RendererConstants::SWAP_CHAIN_COUNT> m_render_cmd;

        std::array<ID3D12Fence*, DX12RendererConstants::SWAP_CHAIN_COUNT> m_fences;
   
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_shadow_map_pipelineState;

        
        CD3DX12_VIEWPORT m_viewport;

        CD3DX12_RECT m_scissorRect;

        std::shared_ptr<gameplay::GamesScene> m_scene;

        std::unique_ptr<UniformBuffer> m_main_camera_uniform;

        std::unique_ptr<DX12DepthBuffer> m_depth_buffer;

        std::unique_ptr<DX12DepthBuffer> m_shadow_map;

        std::unique_ptr<DX12RenderCommndBuffer> m_shadow_map_cmd;
        std::unique_ptr<UniformBuffer> m_shadow_map_camera_uniform;


        std::unordered_map<std::string, std::unique_ptr<DX12Texture>> m_dummy_actor_textures;

        enum {
            CAMERA_UNIFORM_SIZE = 256 //192 byte per buffer(3 buffers),256 for device alignment
        };
        enum {
            DEPTH_BUFFER_WIDTH = 1920
        };
        enum {
            DEPTH_BUFFER_HEIGHT = 1080
        };

        const DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT::DXGI_FORMAT_D16_UNORM;


    private:

        void InitSwapChain();

        void InitFences();

        void InitGraphicsPipelines();

        void WaitForPreviousFrame();

        void RecordGraphicsCmd();

        void InitCmdBuffers();

        void InitCameraUniform();

        void InitDepthBuffer();

        void InitRootSignature();

        void RenderScene(ID3D12GraphicsCommandList*);

        void SetVertexAndIndexBuffer(ID3D12GraphicsCommandList* p_cmd);

    public:
        // Inherited via IModule
        virtual void Init() override;

        virtual void Update() override;

        virtual void Destory() override;

        virtual void SetWindow(HWND hWnd,uint32_t x,uint32_t y, uint32_t width, uint32_t height) override;

        __forceinline virtual void SetCurrentScene(std::shared_ptr<gameplay::GamesScene>) override;

    };//DX12Renderer
}//Renderer


