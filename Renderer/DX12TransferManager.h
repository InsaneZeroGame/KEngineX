#pragma once
#include "DX12GpuDevice.h"
#include <INoCopy.h>
#include "DX12GpuBuffer.h"
#include <EngineConfig.h>


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
            CPU_TO_GPU,
            GPU_TO_GPU,
            GPU_TO_CPU
        }Jobtype;
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




    private:
        DX12TransferManager();
        DX12GpuDevice* m_device;
        DX12GpuBuffer* m_vertex_buffer;
        Microsoft::WRL::ComPtr<ID3D12Heap> m_vertex_index_heap;
        D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
        UINT m_NumBarriersToFlush;
        ID3D12GraphicsCommandList* m_CommandList;

    private:
        void TransitionResource(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate, D3D12_COMMAND_LIST_TYPE);
        void BeginResourceTransition(DX12GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate,D3D12_COMMAND_LIST_TYPE);
        void InsertUAVBarrier(DX12GpuResource& Resource, bool FlushImmediate);
        void FlushResourceBarriers(void);

    };//DX12UploadManager
}//Renderer