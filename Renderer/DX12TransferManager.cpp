#include "DX12TransferManager.h"
Renderer::DX12TransferManager::DX12TransferManager() :
    m_device(DX12GpuDevice::GetGpuDevicePtr()),
    m_NumBarriersToFlush(0),
    m_vertex_buffer(),
    m_upload_buffer()
{
    InitHeaps();
    InitBuffers();
    InitCmdBuffers();
}


void Renderer::DX12TransferManager::InitHeaps()
{
    //Init vertex and index buffer heap.
    {
        D3D12_HEAP_DESC vertex_index_const_desc = {};
        vertex_index_const_desc.Alignment = 64 * 1024;
        vertex_index_const_desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        vertex_index_const_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        vertex_index_const_desc.Properties.CreationNodeMask = 1;
        vertex_index_const_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        vertex_index_const_desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        vertex_index_const_desc.Properties.VisibleNodeMask = 1;
        vertex_index_const_desc.SizeInBytes = KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE;
        ASSERT_SUCCEEDED(m_device->GetDX12Device()->CreateHeap(&vertex_index_const_desc, IID_PPV_ARGS(&m_vertex_buffer.heap)));
    }
   
    //Init upload buffer heap.
    {
        D3D12_HEAP_DESC upload_desc = {};
        upload_desc.Alignment = 64 * 1024;
        upload_desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        upload_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        upload_desc.Properties.CreationNodeMask = 1;
        upload_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        upload_desc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        upload_desc.Properties.VisibleNodeMask = 1;
        upload_desc.SizeInBytes = KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE;
        ASSERT_SUCCEEDED(m_device->GetDX12Device()->CreateHeap(&upload_desc, IID_PPV_ARGS(&m_upload_buffer.heap)));
    }

}

void Renderer::DX12TransferManager::InitBuffers()
{
    //Init Vertex Index Buffer.
    {
        m_vertex_buffer.buffer->CreatePlaced(L"VertexBuffer", m_vertex_buffer.heap.Get(), 0, KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE,D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    }

    //Init Upload Buffer.
    {
        m_upload_buffer.buffer->Create(L"UploadBuffer",KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE);
        D3D12_RANGE l_range = {};
        l_range.Begin = 0;
        l_range.End = 0;
        //Keeps it mapped.
        m_upload_buffer.buffer->GetResource()->Map(0, &l_range, reinterpret_cast<void**>(&m_upload_buffer.data));
    }
}

void Renderer::DX12TransferManager::InitCmdBuffers()
{
    m_CommandList = new DX12RenderCommndBuffer(nullptr);
    m_UploadCommandList = new DX12RenderCommndBuffer(nullptr);

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
        _ASSERT(m_NumBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
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

void Renderer::DX12TransferManager::PrepareToRender()
{

    m_UploadCommandList->Reset();
    m_UploadCommandList->GetDX12CmdList()->CopyBufferRegion(m_vertex_buffer.buffer->GetResource(),0, m_upload_buffer.buffer->GetResource(), 0, KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE);
    m_UploadCommandList->Flush();
    TransitionResource(*m_vertex_buffer.buffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

}

void Renderer::DX12TransferManager::FlushResourceBarriers(void)
{
    if (m_NumBarriersToFlush > 0)
    {

        m_CommandList->Reset();
        m_CommandList->GetDX12CmdList()->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
        m_NumBarriersToFlush = 0;
    }
}

void Renderer::DX12TransferManager::DoOneJob(TransferJob* p_job)
{   

    switch (p_job->type)
    {
    case TransferJob::JobType::UPLOAD_VERTEX_BUFFER:
        UploadDataToVertexBuffer(p_job);  
        break;
    default:
        break;
    }

}

void Renderer::DX12TransferManager::UploadDataToVertexBuffer(TransferJob* p_job)
{
    memcpy(m_upload_buffer.data + m_vertex_buffer.offset, p_job->data, p_job->data_size);
    p_job->gpu_va_address = m_vertex_buffer.buffer->GetGpuVirtualAddress() + m_vertex_buffer.offset;
    m_vertex_buffer.offset += p_job->data_size;
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
    if (m_CommandList) {
        delete m_CommandList;
        m_CommandList = nullptr;
    }
    if (m_UploadCommandList) 
    {
        delete m_UploadCommandList;
        m_CommandList = nullptr;
    }
}
