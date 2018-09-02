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

#include "DX12DepthBuffer.h"
#include "DX12TransferManager.h"

namespace Renderer {

    void DX12DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, 1, 1, Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.DepthStencil.Depth = 1.0f;
        ClearValue.DepthStencil.Stencil = 0;
        ClearValue.Format = Format;
        CreateTextureResource(Name, ResourceDesc, ClearValue, VidMemPtr);
        CreateDerivedViews(Format);
        //DX12TransferManager::GetTransferManager().TransitionResource(*this, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

    }

    void DX12DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t Samples, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, 1, 1, Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        ResourceDesc.SampleDesc.Count = Samples;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.DepthStencil.Depth = 1.0f;
        ClearValue.DepthStencil.Stencil = 0;
        ClearValue.Format = Format;
        CreateTextureResource(Name, ResourceDesc, ClearValue, VidMemPtr);
        CreateDerivedViews(Format);
        //DX12TransferManager::GetTransferManager().TransitionResource(*this, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    

    void DX12DepthBuffer::CreateDerivedViews(DXGI_FORMAT Format)
    {
        ID3D12Resource* Resource = m_pResource.Get();

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = GetDSVFormat(Format);
        if (Resource->GetDesc().SampleDesc.Count == 1)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }

        if (m_hDSV[0].ptr == 0)
        {
            m_hDSV[0] = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV,0);
            m_hDSV[1] = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV,1);
        }

        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        DX12GpuDevice::GetGpuDevice().GetDX12Device()->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[0]);

        dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        //DX12GpuDevice::GetGpuDevice().GetDX12Device()->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[1]);

        DXGI_FORMAT stencilReadFormat = GetStencilFormat(Format);
        if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
        {
            if (m_hDSV[2].ptr == 0)
            {
                m_hDSV[2] = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV,2);
                m_hDSV[3] = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV,3);
            }

            dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            DX12GpuDevice::GetGpuDevice().GetDX12Device()->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[2]);

            dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            DX12GpuDevice::GetGpuDevice().GetDX12Device()->CreateDepthStencilView(Resource, &dsvDesc, m_hDSV[3]);
        }
        else
        {
            m_hDSV[2] = m_hDSV[0];
            m_hDSV[3] = m_hDSV[1];
        }

        if (m_hDepthSRV.ptr == 0)
            m_hDepthSRV = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 4);

        // Create the shader resource view
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = GetDepthFormat(Format);
        if (dsvDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
        {
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = 1;
        }
        else
        {
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        //DX12GpuDevice::GetGpuDevice().GetDX12Device()->CreateShaderResourceView(Resource, &SRVDesc, m_hDepthSRV);

        if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
        {
            if (m_hStencilSRV.ptr == 0)
                m_hStencilSRV = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 4);

            SRVDesc.Format = stencilReadFormat;
            DX12GpuDevice::GetGpuDevice().GetDX12Device()->CreateShaderResourceView(Resource, &SRVDesc, m_hStencilSRV);
        }
    }
}
