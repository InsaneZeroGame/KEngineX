#include "DX12RenderContext.h"
#include "EngineConfig.h"
#include "DX12TransferManager.h"

Renderer::DX12RenderContext::DX12RenderContext()
    :m_device(DX12GpuDevice::GetGpuDevicePtr())
{
    CreateSwapchainResourcesFromDevice();
    InitDepthBuffer();
}

Renderer::DX12RenderContext::~DX12RenderContext()
{
}

void Renderer::DX12RenderContext::PrepareToRender(ID3D12GraphicsCommandList * p_cmd_list, uint8_t current_frame_index)
{
    //Depth Buffer's SRV is the base desc handle in srv heap,shadow map is the second.
    p_cmd_list->SetGraphicsRootDescriptorTable(3, m_depth_buffer->GetDepthSRV().gpu_handle);

    // Indicate that the back buffer will be used as a render target.
    p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain[current_frame_index].resource.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    
    p_cmd_list->ClearRenderTargetView(m_swap_chain[current_frame_index].handle.cpu_handle, clearColor.data(), 0, nullptr);

    p_cmd_list->ClearDepthStencilView(m_depth_buffer->GetDSV().cpu_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
    p_cmd_list->OMSetRenderTargets(1, &m_swap_chain[current_frame_index].handle.cpu_handle, FALSE, &m_depth_buffer->GetDSV().cpu_handle);

}

void Renderer::DX12RenderContext::PrepareToPresent(ID3D12GraphicsCommandList * p_cmd_list, uint8_t current_frame_index)
{
    p_cmd_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain[current_frame_index].resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
