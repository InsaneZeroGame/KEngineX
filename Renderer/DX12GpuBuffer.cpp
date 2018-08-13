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

#include "DX12GpuBuffer.h"

namespace Renderer
{

    void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData)
    {
        GpuResource::Destroy();

        m_ElementCount = NumElements;
        m_ElementSize = ElementSize;
        m_BufferSize = NumElements * ElementSize;

        D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

        m_UsageState = D3D12_RESOURCE_STATE_COMMON;

        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        m_device->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
            &ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource));

        m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

        if (initialData)
            CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);

#ifdef RELEASE
        (name);
#else
        m_pResource->SetName(name.c_str());
#endif

        CreateDerivedViews();
    }

    // Sub-Allocate a buffer out of a pre-allocated heap.  If initial data is provided, it will be copied into the buffer using the default command context.
    void GpuBuffer::CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
        const void* initialData)
    {
        m_ElementCount = NumElements;
        m_ElementSize = ElementSize;
        m_BufferSize = NumElements * ElementSize;

        D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

        m_UsageState = D3D12_RESOURCE_STATE_COMMON;

        m_device->GetDevice()->CreatePlacedResource(pBackingHeap, HeapOffset, &ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource));

        m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

        if (initialData)
            CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);

#ifdef RELEASE
        (name);
#else
        m_pResource->SetName(name.c_str());
#endif

        CreateDerivedViews();

    }

    //void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize,
    //    EsramAllocator&, const void* initialData)
    //{
    //    Create(name, NumElements, ElementSize, initialData);
    //}

    D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t Offset, uint32_t Size) const
    {
        _ASSERT(Offset + Size <= m_BufferSize);

        Size = Math::AlignUp(Size, 16);

        D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
        CBVDesc.BufferLocation = m_GpuVirtualAddress + (size_t)Offset;
        CBVDesc.SizeInBytes = Size;

        D3D12_CPU_DESCRIPTOR_HANDLE hCBV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        g_Device->CreateConstantBufferView(&CBVDesc, hCBV);
        return hCBV;
    }

    D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer(void)
    {
        _ASSERT(m_BufferSize != 0);

        D3D12_RESOURCE_DESC Desc = {};
        Desc.Alignment = 0;
        Desc.DepthOrArraySize = 1;
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        Desc.Flags = m_ResourceFlags;
        Desc.Format = DXGI_FORMAT_UNKNOWN;
        Desc.Height = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        Desc.MipLevels = 1;
        Desc.SampleDesc.Count = 1;
        Desc.SampleDesc.Quality = 0;
        Desc.Width = (UINT64)m_BufferSize;
        return Desc;
    }

}
