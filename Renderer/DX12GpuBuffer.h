//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include "DX12GpuResource.h"
#include <string>
#include "DX12GpuDevice.h"


namespace Renderer
{

    class GpuBuffer : public GpuResource
    {
    public:
        virtual ~GpuBuffer() { Destroy(); }

        // Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
        void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize,
            const void* initialData = nullptr);

        // Create a buffer in ESRAM.  On Windows, ESRAM is not used.
        //void Create( const std::wstring& name, uint32_t NumElements, uint32_t ElementSize,
        //    EsramAllocator& Allocator, const void* initialData = nullptr);

        // Sub-Allocate a buffer out of a pre-allocated heap.  If initial data is provided, it will be copied into the buffer using the default command context.
        void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
            const void* initialData = nullptr);

        const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAV; }
        const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRV; }

        D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView(void) const { return m_GpuVirtualAddress; }

        D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t Offset, uint32_t Size) const;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const;
        D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t BaseVertexIndex = 0) const
        {
            size_t Offset = BaseVertexIndex * m_ElementSize;
            return VertexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize);
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit = false) const;
        D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t StartIndex = 0) const
        {
            size_t Offset = StartIndex * m_ElementSize;
            return IndexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize == 4);
        }

        size_t GetBufferSize() const { return m_BufferSize; }
        uint32_t GetElementCount() const { return m_ElementCount; }
        uint32_t GetElementSize() const { return m_ElementSize; }

    protected:

        GpuBuffer(void) : m_BufferSize(0), m_ElementCount(0), m_ElementSize(0),
            m_device(nullptr)
        {
            m_ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            m_UAV.ptr = 0;
            m_SRV.ptr = 0;
        }

        GpuBuffer(DX12GpuDevice* p_device) : m_BufferSize(0), m_ElementCount(0), m_ElementSize(0),
            m_device(p_device)
        {
            m_ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            m_UAV.ptr = 0;
            m_SRV.ptr = 0;
        }

        D3D12_RESOURCE_DESC DescribeBuffer(void);
        virtual void CreateDerivedViews(void) = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE m_UAV;
        D3D12_CPU_DESCRIPTOR_HANDLE m_SRV;

        size_t m_BufferSize;
        uint32_t m_ElementCount;
        uint32_t m_ElementSize;
        D3D12_RESOURCE_FLAGS m_ResourceFlags;
        DX12GpuDevice* m_device;
    };

    inline D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const
    {
        D3D12_VERTEX_BUFFER_VIEW VBView;
        VBView.BufferLocation = m_GpuVirtualAddress + Offset;
        VBView.SizeInBytes = Size;
        VBView.StrideInBytes = Stride;
        return VBView;
    }

    inline D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit) const
    {
        D3D12_INDEX_BUFFER_VIEW IBView;
        IBView.BufferLocation = m_GpuVirtualAddress + Offset;
        IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        IBView.SizeInBytes = Size;
        return IBView;
    }

}//Renderer