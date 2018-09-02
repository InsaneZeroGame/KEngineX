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
            UPLOAD_VERTEX_BUFFER,
        }Jobtype;

        JobType type;
        void* data;
        uint64_t data_size;
        bool job_done;
        bool retain;
        uint64_t gpu_va_address;

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

    private:
        DX12TransferManager();
        DX12GpuDevice* m_device;
        std::unique_ptr<VertexIndexBuffer> m_vertex_buffer;
        std::unique_ptr<UniformBuffer> m_upload_buffer;
        D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
        UINT m_NumBarriersToFlush;
        DX12RenderCommndBuffer* m_CommandList;
        DX12RenderCommndBuffer* m_UploadCommandList;

        std::queue<TransferJob*> m_jobs;
    private:
        void InitBuffers();
        void InitCmdBuffers();
        void BeginResourceTransition(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate,D3D12_COMMAND_LIST_TYPE);
        void InsertUAVBarrier(DX12GpuResource& Resource, bool FlushImmediate);
        void FlushResourceBarriers(void);
        void DoOneJob(TransferJob*);
        void UploadDataToVertexBuffer(TransferJob* p_job);
    };//DX12UploadManager
}//Renderer