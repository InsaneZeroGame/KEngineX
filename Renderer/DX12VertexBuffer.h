#pragma once
#include "DX12GpuBuffer.h"


namespace Renderer
{
    struct VertexIndexBuffer final: public DX12GpuBuffer
    {
        Microsoft::WRL::ComPtr<ID3D12Heap> heap;
        uint64_t offset = 0;

        VertexIndexBuffer(uint64_t p_size) : DX12GpuBuffer()
        {
            m_BufferSize = p_size;

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
                vertex_index_const_desc.SizeInBytes = m_BufferSize;
                ASSERT_SUCCEEDED(m_device->GetDX12Device()->CreateHeap(&vertex_index_const_desc, IID_PPV_ARGS(&heap)));
            }

            //Init Vertex Index Buffer.
            {
                CreatePlaced(L"VertexBuffer", heap.Get(), 0, m_BufferSize, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
            }
        }

        ~VertexIndexBuffer()
        {
            
        }

    };
}
