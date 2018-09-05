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

#include "DX12PixelBuffer.h"

namespace Renderer
{

    class DX12DepthBuffer : public DX12PixelBuffer
    {
    public:
        DX12DepthBuffer(float ClearDepth = 0.0f, uint8_t ClearStencil = 0)
            : m_ClearDepth(ClearDepth), m_ClearStencil(ClearStencil)
        {
            m_hDSV[0] = {};
            m_hDSV[1] = {};
            m_hDSV[2] = {};
            m_hDSV[3] = {};
            m_hDepthSRV = {};
            m_hStencilSRV = {};
        }

        // Create a depth buffer.  If an address is supplied, memory will not be allocated.
        // The vmem address allows you to alias buffers (which can be especially useful for
        // reusing ESRAM across a frame.)
        void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
            D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = 0);

        // Create a depth buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
        // this functions the same as Create() without a video address.

        void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumSamples, DXGI_FORMAT Format,
            D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = 0);

        // Get pre-created CPU-visible descriptor handles
        const DescriptorHandle& GetDSV() const { return m_hDSV[0]; }
        const DescriptorHandle& GetDSV_DepthReadOnly() const { return m_hDSV[1]; }
        const DescriptorHandle& GetDSV_StencilReadOnly() const { return m_hDSV[2]; }
        const DescriptorHandle& GetDSV_ReadOnly() const { return m_hDSV[3]; }
        const DescriptorHandle& GetDepthSRV() const { return m_hDepthSRV; }
        const DescriptorHandle& GetStencilSRV() const { return m_hStencilSRV; }

        float GetClearDepth() const { return m_ClearDepth; }
        uint8_t GetClearStencil() const { return m_ClearStencil; }

    private:

        void CreateDerivedViews(DXGI_FORMAT Format);

        float m_ClearDepth;
        uint8_t m_ClearStencil;
        DescriptorHandle m_hDSV[4];
        DescriptorHandle m_hDepthSRV;
        DescriptorHandle m_hStencilSRV;
    };
}