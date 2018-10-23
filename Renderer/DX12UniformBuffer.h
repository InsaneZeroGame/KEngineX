#pragma once
#include "DX12GpuBuffer.h"

namespace Renderer
{
    struct UniformBuffer final : public DX12GpuBuffer 
    {
        
        UniformBuffer(uint64_t p_size) : 
            DX12GpuBuffer()
        {
            m_BufferSize = p_size;
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
                upload_desc.SizeInBytes = m_BufferSize;
                ASSERT_SUCCEEDED(m_device->GetDX12Device()->CreateHeap(&upload_desc, IID_PPV_ARGS(&heap)));
            }

            //Init Upload Buffer.
            {
                Create(L"UploadBuffer", m_BufferSize);
                D3D12_RANGE l_range = {};
                l_range.Begin = 0;
                l_range.End = 0;
                //Keeps it mapped.
                GetResource()->Map(0, &l_range, reinterpret_cast<void**>(&data));
            }

       
        }

        ~UniformBuffer()
        {
            
        }

        __forceinline void UpdateBuffer(const void* src_data,size_t size,size_t offset = 0)
        {
            memcpy(data + offset, src_data, size);
        };

    private:
        Microsoft::WRL::ComPtr<ID3D12Heap> heap;
        uint8_t* data;

    };//Class UniformBuffer
}//Namespace Renderer