#include "DX12TransferManager.h"
Renderer::DX12TransferManager::DX12TransferManager():
    m_device(DX12GpuDevice::GetGpuDevicePtr()),
    m_NumBarriersToFlush(0)
{
    D3D12_HEAP_DESC vertex_index_const_desc = {};
    vertex_index_const_desc.Alignment = 64 * 1024;
    vertex_index_const_desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
    vertex_index_const_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    vertex_index_const_desc.Properties.CreationNodeMask = 1;
    vertex_index_const_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    vertex_index_const_desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    vertex_index_const_desc.Properties.VisibleNodeMask = 1;
    vertex_index_const_desc.SizeInBytes = 1;
    ASSERT_SUCCEEDED(m_device->GetDevice()->CreateHeap(&vertex_index_const_desc, IID_PPV_ARGS(&m_vertex_index_heap)));
}


void Renderer::DX12TransferManager::TransitionResource(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate, D3D12_COMMAND_LIST_TYPE p_type)
{
    D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

    if (p_type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        _ASSERT((OldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == OldState);
        _ASSERT((NewState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == NewState);
    }

    if (OldState != NewState)
    {
        _ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = Resource.GetResource();
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;

        // Check to see if we already started the transition
        if (NewState == Resource.m_TransitioningState)
        {
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
        }
        else
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        Resource.m_UsageState = NewState;
    }
    else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        InsertUAVBarrier(Resource, FlushImmediate);

    if (FlushImmediate || m_NumBarriersToFlush == 16)
        FlushResourceBarriers();
}

void Renderer::DX12TransferManager::FlushResourceBarriers(void)
{
    if (m_NumBarriersToFlush > 0)
    {
        m_CommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
        m_NumBarriersToFlush = 0;
    }
}

void Renderer::DX12TransferManager::BeginResourceTransition(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate,D3D12_COMMAND_LIST_TYPE p_type)
{
    // If it's already transitioning, finish that transition
    if (Resource.m_TransitioningState != (D3D12_RESOURCE_STATES)-1)
        TransitionResource(Resource, Resource.m_TransitioningState,true,p_type);

    D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

    if (OldState != NewState)
    {
        ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = Resource.GetResource();
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;

        BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

        Resource.m_TransitioningState = NewState;
    }

    if (FlushImmediate || m_NumBarriersToFlush == 16)
        FlushResourceBarriers();
}

void Renderer::DX12TransferManager::InsertUAVBarrier(DX12GpuResource& Resource, bool FlushImmediate)
{
    ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
    D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

    BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    BarrierDesc.UAV.pResource = Resource.GetResource();

    if (FlushImmediate)
        FlushResourceBarriers();
}

Renderer::DX12TransferManager::~DX12TransferManager()
{

}
