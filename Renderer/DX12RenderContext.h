#pragma once
#include "DX12GpuDevice.h"
#include "DX12DepthBuffer.h"
#include "DX12ColorBuffer.h"
#include "DX12RendererConfig.h"
#include "DX12TransferManager.h"

const DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT::DXGI_FORMAT_D16_UNORM;

namespace Renderer
{
    class DX12RenderContext
    {
    public:
        DX12RenderContext();
        ~DX12RenderContext();

        void PreDeferredPass(ID3D12GraphicsCommandList* p_cmd_list,uint8_t current_frame_index);

        void PreShadowMapping(ID3D12GraphicsCommandList* p_cmd_list);

        __forceinline void PostShadowMapping() const
        {
            //Expose shadow map to shader.
            DX12TransferManager::GetTransferManager().TransitionResource(*m_shadow_map, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

        };

        void PrePresent(ID3D12GraphicsCommandList* p_cmd_list, uint8_t current_frame_index);

        __forceinline const DX12ColorBuffer* const GetColorAttachment(uint8_t p_index) const
        {
            assert(p_index < m_color_attachments.size());
            return m_color_attachments[p_index];
        }

        void AddColorAttachment(const std::wstring& Name, uint32_t Width, uint32_t Height, 
            DXGI_FORMAT Format, uint32_t NumMips = 1, D3D12_GPU_VIRTUAL_ADDRESS VidMem = 0);
        void AddColorAttachment(const ColorAttachmentDescriptor& p_desc);
    private:
        std::vector<DX12ColorBuffer*> m_color_attachments;

        std::unique_ptr<DX12DepthBuffer> m_depth_buffer;

        std::unique_ptr<DX12DepthBuffer> m_shadow_map;

        
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

        void InitShadowMap();

        void InitColorBuffers();


    };
}