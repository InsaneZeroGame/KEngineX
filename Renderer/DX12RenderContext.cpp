#include "DX12RenderContext.h"
#include "EngineConfig.h"

Renderer::DX12RenderContext::DX12RenderContext()
    :m_device(DX12GpuDevice::GetGpuDevicePtr())
{
    CreateSwapchainResourcesFromDevice();
    InitDepthBuffer();
    InitShadowMap();
    InitColorBuffers();
}



void Renderer::DX12RenderContext::PreDeferredPass(ID3D12GraphicsCommandList * p_cmd_list, uint8_t current_frame_index)
{
    //Depth Buffer's SRV is the base desc handle in srv heap,shadow map is the second.
    p_cmd_list->SetGraphicsRootDescriptorTable(3, m_depth_buffer->GetDepthSRV().gpu_handle);

    // Indicate that the back buffer will be used as a render target.
    p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain[current_frame_index].resource.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    
    p_cmd_list->ClearRenderTargetView(m_swap_chain[current_frame_index].handle.cpu_handle, clearColor.data(), 0, nullptr);

    p_cmd_list->ClearDepthStencilView(m_depth_buffer->GetDSV().cpu_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtv_handles
    {
        m_swap_chain[current_frame_index].handle.cpu_handle,//Swap Chain
        m_color_attachments[0]->GetRTV().cpu_handle,//Normal Attachment
    };

    p_cmd_list->OMSetRenderTargets(rtv_handles.size(), rtv_handles.data(), FALSE, &m_depth_buffer->GetDSV().cpu_handle);

}

void Renderer::DX12RenderContext::PreShadowMapping(ID3D12GraphicsCommandList * p_cmd_list)
{
    p_cmd_list->ClearDepthStencilView(m_shadow_map->GetDSV().cpu_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
    p_cmd_list->OMSetRenderTargets(0, nullptr, FALSE, &m_shadow_map->GetDSV().cpu_handle);
}


void Renderer::DX12RenderContext::PrePresent(ID3D12GraphicsCommandList * p_cmd_list, uint8_t current_frame_index)
{
    p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain[current_frame_index].resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    DX12TransferManager::GetTransferManager().TransitionResource(*m_shadow_map, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

}

void Renderer::DX12RenderContext::AddColorAttachment(const std::wstring& Name, uint32_t Width, uint32_t Height, 
    DXGI_FORMAT Format, uint32_t NumMips, D3D12_GPU_VIRTUAL_ADDRESS VidMem)
{
    auto l_new_attachemnt = new DX12ColorBuffer();
    m_color_attachments.push_back(l_new_attachemnt);
    l_new_attachemnt->Create(Name, Width, Height, NumMips, Format);
}

void Renderer::DX12RenderContext::AddColorAttachment(const ColorAttachmentDescriptor & p_desc)
{
    AddColorAttachment(p_desc.Name,p_desc.Width,p_desc.Height,p_desc.Format,p_desc.NumMips,p_desc.VidMem);
}


void Renderer::DX12RenderContext::CreateSwapchainResourcesFromDevice()
{
    for (UINT n = 0; n < DX12RendererConstants::SWAP_CHAIN_COUNT; n++)
    {
        m_swap_chain[n].handle = m_device->GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_device->GetSwapChain()->GetBuffer(n, IID_PPV_ARGS(&m_swap_chain[n].resource));
        m_device->GetDX12Device()->CreateRenderTargetView(m_swap_chain[n].resource.Get(), nullptr, m_swap_chain[n].handle.cpu_handle);
    }

    
}

void Renderer::DX12RenderContext::InitDepthBuffer()
{
    //ToDo:May gain performance using DENY_SHADER_ACCESS.
    m_depth_buffer = std::unique_ptr<DX12DepthBuffer>(new DX12DepthBuffer());
    m_depth_buffer->Create(L"DepthBuffer", KEngineConstants::WINDOW_WIDTH, KEngineConstants::WINDOW_HEIGHT, DEPTH_BUFFER_FORMAT);
    DX12TransferManager::GetTransferManager().TransitionResource(*m_depth_buffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

}

void Renderer::DX12RenderContext::InitShadowMap()
{
    m_shadow_map = std::unique_ptr<DX12DepthBuffer>(new DX12DepthBuffer());
    m_shadow_map->Create(L"ShadowMap", DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT, DEPTH_BUFFER_FORMAT);
    DX12TransferManager::GetTransferManager().TransitionResource(*m_shadow_map, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Renderer::DX12RenderContext::InitColorBuffers()
{
    //Init color attachments based on config in render config
    for (const auto& l_attachment_desc : COLOR_ATTACHMENT_CONFIG)
    {
        AddColorAttachment(l_attachment_desc);
    }
}

Renderer::DX12RenderContext::~DX12RenderContext()
{
    for (auto l_attachment : m_color_attachments)
    {
        if (l_attachment)
        {
            delete l_attachment;
            l_attachment = nullptr;
        }
    }
}
