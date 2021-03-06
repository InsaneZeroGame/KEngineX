#include "DX12TransferManager.h"
Renderer::DX12TransferManager::DX12TransferManager() :
    m_device(DX12GpuDevice::GetGpuDevicePtr()),
    m_NumBarriersToFlush(0),
    m_vertex_buffer(nullptr),
    m_index_buffer(nullptr),
    m_vertex_upload_buffer(nullptr),
    m_index_upload_buffer(nullptr)
{
    InitBuffers();
    InitCmdBuffers();
}

void Renderer::DX12TransferManager::InitBuffers()
{
    m_index_upload_buffer = std::unique_ptr<UniformBuffer>(new UniformBuffer(KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE));
    m_vertex_upload_buffer = std::unique_ptr<UniformBuffer>(new UniformBuffer(KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE));
    m_texture_upload_buffer = std::unique_ptr<UniformBuffer>(new UniformBuffer(KEngineConstants::TEXTURE_UPLOAD_BUFFER_SIZE_IN_BYTE));
    m_vertex_buffer = std::unique_ptr<VertexIndexBuffer>(new VertexIndexBuffer(L"VertexBuffer",KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE));
    m_index_buffer = std::unique_ptr<VertexIndexBuffer>(new VertexIndexBuffer(L"IndexBuffer", KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE));

}

void Renderer::DX12TransferManager::InitCmdBuffers()
{
    m_CommandList = std::unique_ptr<DX12RenderCommndBuffer>(new DX12RenderCommndBuffer(nullptr));
    m_UploadCommandList = std::unique_ptr<DX12RenderCommndBuffer>(new DX12RenderCommndBuffer(nullptr));
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
    m_UploadCommandList->GetDX12CmdList()->CopyBufferRegion(m_vertex_buffer->GetResource(),0, m_vertex_upload_buffer->GetResource(), 0, KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE);
    m_UploadCommandList->GetDX12CmdList()->CopyBufferRegion(m_index_buffer->GetResource(), 0, m_index_upload_buffer->GetResource(), 0, KEngineConstants::VERTEX_INDEX_BUFFER_SIZE_IN_BYTE);
    m_UploadCommandList->Flush();
    TransitionResource(*m_vertex_buffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
    TransitionResource(*m_index_buffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

}

void Renderer::DX12TransferManager::FlushResourceBarriers(void)
{
    if (m_NumBarriersToFlush > 0)
    {

        m_CommandList->Reset();
        m_CommandList->GetDX12CmdList()->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
        m_CommandList->Flush();
        m_NumBarriersToFlush = 0;
    }
}

void Renderer::DX12TransferManager::DoOneJob(TransferJob* p_job)
{   

    switch (p_job->type)
    {
    case TransferJob::JobType::UPLOAD_VERTEX:
        UploadDataToVertexBuffer(p_job);  
        break;
    case TransferJob::JobType::UPLOAD_INDEX:
        UploadDataToIndexBuffer(p_job);
        break;
    case TransferJob::JobType::UPLOAD_TEXTURE:
        UploadTexture(p_job);
        break;
    default:
        break;
    }

}

void Renderer::DX12TransferManager::UploadDataToVertexBuffer(TransferJob* p_job)
{
    m_vertex_upload_buffer->UpdateBuffer(p_job->data, p_job->data_size, m_vertex_buffer->offset);
    m_vertex_buffer->offset += p_job->data_size;
    p_job->vertex_offset = m_vertex_id_offset;
    m_vertex_id_offset += (p_job->vertex_count);
}

void Renderer::DX12TransferManager::UploadDataToIndexBuffer(TransferJob * p_job)
{
    m_index_upload_buffer->UpdateBuffer(p_job->data, p_job->data_size, m_index_buffer->offset);
    m_index_buffer->offset += p_job->data_size;
    p_job->index_offet = m_index_id_offset;
    m_index_id_offset += p_job->index_count;
}

void Renderer::DX12TransferManager::UploadTexture(TransferJob * p_job)
{
    //system memry data to upload buffer
    m_UploadCommandList->Reset();
    D3D12_SUBRESOURCE_DATA texResource;
    texResource.pData = p_job->data;
    texResource.RowPitch = p_job->RowPitch;
    texResource.SlicePitch = p_job->SlicePitch;
    m_texture_upload_buffer->UpdateBuffer(p_job->data, p_job->data_size);

    //upload buffer to texture resource
    UpdateSubresources(m_UploadCommandList->GetDX12CmdList(), p_job->dest_res->GetResource(), m_texture_upload_buffer->GetResource(), 0, 0, 1, &texResource);
    m_UploadCommandList->Flush();
    TransitionResource(*p_job->dest_res, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
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
