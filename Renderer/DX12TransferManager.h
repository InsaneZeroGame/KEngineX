#pragma once
#include "DX12GpuDevice.h"
#include <INoCopy.h>
#include "DX12UniformBuffer.h"
#include "DX12VertexBuffer.h"
#include <EngineConfig.h>
#include "DX12ComandBuffer.h"
#include <queue>

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace Renderer 
{

    struct TransferJob
    {
        typedef enum JobType
        {
            UPLOAD_VERTEX,
            UPLOAD_INDEX,
            UPLOAD_TEXTURE,

        }Jobtype;

        uint64_t vertex_count = 0;
        uint64_t index_count = 0;
        uint64_t vertex_offset = 0;
        uint64_t index_offet = 0;
        JobType type;
        const void* data;
        uint64_t data_size;
        bool job_done;
        bool retain;
        //uint64_t gpu_va_address;
        //For Texture Transfer
        uint64_t RowPitch;
        uint64_t SlicePitch;
        DX12GpuResource* dest_res;
    };

    class DX12TransferManager final : public KFramework::INoCopy
    {
    public:

        ~DX12TransferManager();

        __forceinline static DX12TransferManager& GetTransferManager()
        {
            static DX12TransferManager l_device;
            return l_device;
        }

        __forceinline static DX12TransferManager* GetTransferManagerPtr()
        {
            return &GetTransferManager();
        }

        __forceinline void AddTransferJob(TransferJob* job,bool flush_imediate = false)
        {
            if (flush_imediate) {
                DoOneJob(job);
            }
            else
                m_jobs.push(job);
        }

        void PrepareToRender();

        void TransitionResource(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate, D3D12_COMMAND_LIST_TYPE);

        __forceinline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()
        {
            return m_vertex_buffer->VertexBufferView(0, static_cast<uint32_t>(m_vertex_buffer->GetBufferSize()),static_cast<uint32_t>(sizeof(Vertex)));
        }

        __forceinline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView()
        {
            return m_index_buffer->IndexBufferView(0,static_cast<uint32_t>(m_index_buffer->GetBufferSize()),true);
        }

        //vertex id offset in the vertex buffer.
        uint64_t m_vertex_id_offset = 0;
        //Index id offset in the index buffer.
        uint64_t m_index_id_offset = 0;


    private:
        DX12TransferManager();
        DX12GpuDevice* m_device;
        std::unique_ptr<VertexIndexBuffer> m_vertex_buffer;
        
        std::unique_ptr<VertexIndexBuffer> m_index_buffer;

        std::unique_ptr<UniformBuffer> m_vertex_upload_buffer;
        
        std::unique_ptr<UniformBuffer> m_index_upload_buffer;

        std::unique_ptr<UniformBuffer> m_texture_upload_buffer;

        D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
        UINT m_NumBarriersToFlush;
        std::unique_ptr<DX12RenderCommndBuffer> m_CommandList;
        std::unique_ptr<DX12RenderCommndBuffer> m_UploadCommandList;

        std::queue<TransferJob*> m_jobs;
    private:
        void InitBuffers();
        void InitCmdBuffers();
        void BeginResourceTransition(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate,D3D12_COMMAND_LIST_TYPE);
        void InsertUAVBarrier(DX12GpuResource& Resource, bool FlushImmediate);
        void FlushResourceBarriers(void);
        void DoOneJob(TransferJob*);
        void UploadDataToVertexBuffer(TransferJob* p_job);
        void UploadDataToIndexBuffer(TransferJob* p_job);
        void UploadTexture(TransferJob* p_job);
    };//DX12UploadManager
}//Renderer