#pragma once
#include "DX12GpuDevice.h"
#include "DX12DepthBuffer.h"
const DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT::DXGI_FORMAT_D16_UNORM;

namespace Renderer
{
    class DX12RenderContext
    {
    public:
        DX12RenderContext();
        ~DX12RenderContext();

        void PrepareToRender(ID3D12GraphicsCommandList* p_cmd_list,uint8_t current_frame_index);

        void PrepareToPresent(ID3D12GraphicsCommandList* p_cmd_list, uint8_t current_frame_index);

        


    private:
        std::unique_ptr<DX12DepthBuffer> m_depth_buffer;
        
        DX12GpuDevice* m_device;

        std::array<float,4> clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };

        struct DX12SwapChain
        {
            DescriptorHandle handle;

            Microsoft::WRL::ComPtr<ID3D12Resource> resource;

        }m_swap_chain[DX12RendererConstants::SWAP_CHAIN_COUNT];


    private:
        void CreateSwapchainResourcesFromDevice();

        void InitDepthBuffer();


    };
}