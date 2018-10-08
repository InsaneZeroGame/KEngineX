#include "DX12GpuDevice.h"
#include "DXSampleHelper.h"
#include <EngineConfig.h>
#include <Win32Application.h>

Renderer::DX12GpuDevice::DX12GpuDevice():
    m_useWarpDevice(false),
    m_desc_heaps_handle_count({})
{

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&m_adapter)));

        ThrowIfFailed(D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(hardwareAdapter.As(&m_adapter));

        ThrowIfFailed(D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        ));
    }

    //Init CommandQueue.
    {

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    }

    //Init SwapChain.
    {
        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = DX12RendererConstants::SWAP_CHAIN_COUNT;
        swapChainDesc.Width = KEngineConstants::WINDOW_WIDTH;
        swapChainDesc.Height = KEngineConstants::WINDOW_HEIGHT;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
            Win32Application::GetHwnd(),
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        // This sample does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

        ThrowIfFailed(swapChain.As(&m_swapChain));
    }

    //Allocate Descriptor Heaps
    {
        AllocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        AllocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        AllocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
   

    m_device->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_cmd_flush_fence));
}

void Renderer::DX12GpuDevice::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
    ComPtr<IDXGIAdapter1> adapter;
    *ppAdapter = nullptr;

    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            // If you want a software adapter, pass in "/warp" on the command line.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    *ppAdapter = adapter.Detach();
}


void Renderer::DX12GpuDevice::AllocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE p_type)
{
    D3D12_DESCRIPTOR_HEAP_DESC Desc;
    Desc.Type = p_type;
    Desc.NumDescriptors = DESCRIPTOR_HANDLE_MAX_NUM;
    Desc.Flags = p_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE  :  D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Desc.NodeMask = 1;

    ASSERT_SUCCEEDED(m_device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&m_desc_heaps[p_type])));
}

void Renderer::DX12GpuDevice::WaitForGPU()
{
    if (m_cmd_flush_fence->GetCompletedValue() < m_cmd_flush_fence_value)
    {
        m_cmd_flush_fence->SetEventOnCompletion(m_cmd_flush_fence_value, m_cmd_flush_wait_event);
        WaitForSingleObjectEx(m_cmd_flush_wait_event, FENCE_WAIT_TIME_INFINITY, false);
    }
    m_cmd_flush_fence_value++;
}

void Renderer::DX12GpuDevice::FlushCmd(ID3D12CommandList** pp_cmd, uint32_t count)
{
    m_commandQueue->ExecuteCommandLists(count, pp_cmd);
    m_commandQueue->Signal(m_cmd_flush_fence.Get(), m_cmd_flush_fence_value);
    WaitForGPU();
}

Renderer::DescriptorHandle Renderer::DX12GpuDevice::GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE p_type,uint32_t* index)
{
    assert(m_desc_heaps_handle_count[p_type] <= DESCRIPTOR_HANDLE_MAX_NUM && "Handle Num Exceeds max num");

    if (index)
    {
        *index = m_desc_heaps_handle_count[p_type];
    }

    DescriptorHandle l_handle = {};
    l_handle.cpu_handle.ptr = m_desc_heaps[p_type]->GetCPUDescriptorHandleForHeapStart().ptr + m_device->GetDescriptorHandleIncrementSize(p_type) * m_desc_heaps_handle_count[p_type];
    l_handle.gpu_handle.ptr = m_desc_heaps[p_type]->GetGPUDescriptorHandleForHeapStart().ptr + m_device->GetDescriptorHandleIncrementSize(p_type) * m_desc_heaps_handle_count[p_type];
    m_desc_heaps_handle_count[p_type]++;

    return l_handle;
}

Renderer::DX12GpuDevice::~DX12GpuDevice()
{
}
