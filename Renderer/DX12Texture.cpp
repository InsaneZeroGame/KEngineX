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
// Author(s):  James Stanard 
//             Alex Nankervis
//

#include "DX12Texture.h"
#include <Utility.h>
//#include "DDSTextureLoader.h"
#include <map>
#include <thread>
#include "DX12GpuDevice.h"
#include <ppl.h>
#include "DX12TransferManager.h"

using namespace std;

namespace Renderer
{
    size_t BitsPerPixel(_In_ DXGI_FORMAT fmt);

    static UINT BytesPerPixel(DXGI_FORMAT Format);

    void DX12Texture::Create(std::wstring p_name, size_t Pitch, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData)
    {
        m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = Width;
        texDesc.Height = (UINT)Height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = Format;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        ASSERT_SUCCEEDED(DX12GpuDevice::GetGpuDevicePtr()->GetDX12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
            m_UsageState, nullptr, IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())));

        m_pResource->SetName(p_name.c_str());

        

        TransferJob l_job = {};
        l_job.type = TransferJob::JobType::UPLOAD_TEXTURE;
        l_job.data = InitialData;
        l_job.RowPitch = Pitch * BytesPerPixel(Format) * Width;
        l_job.SlicePitch = l_job.RowPitch * Height;
        l_job.data_size = l_job.SlicePitch;
        l_job.dest_res = this;
        DX12TransferManager::GetTransferManager().AddTransferJob(&l_job,true);

        if (m_handle.cpu_handle.ptr == 0)
            m_handle = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,&m_descriptor_heap_index);
        DX12GpuDevice::GetGpuDevicePtr()->GetDX12Device()->CreateShaderResourceView(m_pResource.Get(), nullptr, m_handle.cpu_handle);
    }

    void DX12Texture::CreateTGAFromMemory(const void* _filePtr, size_t, bool sRGB)
    {
        const uint8_t* filePtr = (const uint8_t*)_filePtr;

        // Skip first two bytes
        filePtr += 2;

        /*uint8_t imageTypeCode =*/ *filePtr++;

        // Ignore another 9 bytes
        filePtr += 9;

        uint16_t imageWidth = *(uint16_t*)filePtr;
        filePtr += sizeof(uint16_t);
        uint16_t imageHeight = *(uint16_t*)filePtr;
        filePtr += sizeof(uint16_t);
        uint8_t bitCount = *filePtr++;

        // Ignore another byte
        filePtr++;

        uint32_t* formattedData = new uint32_t[imageWidth * imageHeight];
        uint32_t* iter = formattedData;

        uint8_t numChannels = bitCount / 8;
        uint32_t numBytes = imageWidth * imageHeight * numChannels;

        switch (numChannels)
        {
        default:
            break;
        case 3:
            for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 3)
            {
                *iter++ = 0xff000000 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
                filePtr += 3;
            }
            break;
        case 4:
            for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 4)
            {
                *iter++ = filePtr[3] << 24 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
                filePtr += 4;
            }
            break;
        }

        Create(L"TGATexture",imageWidth, imageHeight, sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, formattedData);

        delete[] formattedData;
    }

    bool DX12Texture::CreateDDSFromMemory(const void* filePtr, size_t fileSize, bool sRGB)
    {
        assert(0);
        //if (m_hCpuDescriptorHandle.ptr == 0)
        //    m_hCpuDescriptorHandle = DX12GpuDevice::GetGpuDevice().GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).cpu_handle;
        //
        //HRESULT hr = CreateDDSTextureFromMemory(DX12GpuDevice::GetGpuDevicePtr()->GetDX12Device(),
        //    (const uint8_t*)filePtr, fileSize, 0, sRGB, &m_pResource, m_hCpuDescriptorHandle);
        //
        //return SUCCEEDED(hr);
        return true;
    }

    void DX12Texture::CreatePIXImageFromMemory(const void* memBuffer, size_t fileSize)
    {
        struct Header
        {
            DXGI_FORMAT Format;
            uint32_t Pitch;
            uint32_t Width;
            uint32_t Height;
        };
        const Header& header = *(Header*)memBuffer;

        ASSERT(fileSize >= header.Pitch * BytesPerPixel(header.Format) * header.Height + sizeof(Header),
            "Raw PIX image dump has an invalid file size");

        Create(L"PIXTexture",header.Pitch, header.Width, header.Height, header.Format, (uint8_t*)memBuffer + sizeof(Header));
    }

    namespace TextureManager
    {
        wstring s_RootPath = L"";
        map< wstring, unique_ptr<ManagedTexture> > s_TextureCache;

        void Initialize(const std::wstring& TextureLibRoot)
        {
            s_RootPath = TextureLibRoot;
        }

        void Shutdown(void)
        {
            s_TextureCache.clear();
        }

        pair<ManagedTexture*, bool> FindOrLoadTexture(const wstring& fileName)
        {
            static mutex s_Mutex;
            lock_guard<mutex> Guard(s_Mutex);

            auto iter = s_TextureCache.find(fileName);

            // If it's found, it has already been loaded or the load process has begun
            if (iter != s_TextureCache.end())
                return make_pair(iter->second.get(), false);

            ManagedTexture* NewTexture = new ManagedTexture(fileName);
            s_TextureCache[fileName].reset(NewTexture);

            // This was the first time it was requested, so indicate that the caller must read the file
            return make_pair(NewTexture, true);
        }

        const DX12Texture& GetBlackTex2D(void)
        {
            auto ManagedTex = FindOrLoadTexture(L"DefaultBlackTexture");

            ManagedTexture* ManTex = ManagedTex.first;
            const bool RequestsLoad = ManagedTex.second;

            if (!RequestsLoad)
            {
                ManTex->WaitForLoad();
                return *ManTex;
            }

            uint32_t BlackPixel = 0;
            ManTex->Create(L"DefaultBlackTexture",1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackPixel);
            return *ManTex;
        }

        const DX12Texture& GetWhiteTex2D(void)
        {
            auto ManagedTex = FindOrLoadTexture(L"DefaultWhiteTexture");

            ManagedTexture* ManTex = ManagedTex.first;
            const bool RequestsLoad = ManagedTex.second;

            if (!RequestsLoad)
            {
                ManTex->WaitForLoad();
                return *ManTex;
            }

            uint32_t WhitePixel = 0xFFFFFFFFul;
            ManTex->Create(L"DefaultWhiteTexture",1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhitePixel);
            return *ManTex;
        }

        const DX12Texture& GetMagentaTex2D(void)
        {
            auto ManagedTex = FindOrLoadTexture(L"DefaultMagentaTexture");

            ManagedTexture* ManTex = ManagedTex.first;
            const bool RequestsLoad = ManagedTex.second;

            if (!RequestsLoad)
            {
                ManTex->WaitForLoad();
                return *ManTex;
            }

            uint32_t MagentaPixel = 0x00FF00FF;
            ManTex->Create(L"DefaultMagentaTexture",1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &MagentaPixel);
            return *ManTex;
        }

    } // namespace TextureManager

    void ManagedTexture::WaitForLoad(void) const
    {
        volatile D3D12_CPU_DESCRIPTOR_HANDLE& VolHandle = (volatile D3D12_CPU_DESCRIPTOR_HANDLE&)m_handle;
        volatile bool& VolValid = (volatile bool&)m_IsValid;
        while (VolHandle.ptr == 0 && VolValid)
            this_thread::yield();
    }

    void ManagedTexture::SetToInvalidTexture(void)
    {
        //m_handle = TextureManager::GetMagentaTex2D().GetSRV();
        m_IsValid = false;
    }

    const ManagedTexture* TextureManager::LoadFromFile(const std::wstring& fileName, bool sRGB)
    {
        //std::wstring CatPath = fileName;
        //
        //const ManagedTexture* Tex = LoadDDSFromFile(CatPath + L".dds", sRGB);
        //if (!Tex->IsValid())
        //    Tex = LoadTGAFromFile(CatPath + L".tga", sRGB);
        //
        //return Tex;
        return nullptr;
    }

    static size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
    {
        switch (fmt)
        {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2:
            return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11:
            return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 4;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            return 0;
        }
    }

    static UINT BytesPerPixel(DXGI_FORMAT Format)
    {
        return (UINT)BitsPerPixel(Format) / 8;
    };

    //const ManagedTexture* TextureManager::LoadDDSFromFile(const std::wstring& fileName, bool sRGB)
    //{
    //    auto ManagedTex = FindOrLoadTexture(fileName);
    //
    //    ManagedTexture* ManTex = ManagedTex.first;
    //    const bool RequestsLoad = ManagedTex.second;
    //
    //    if (!RequestsLoad)
    //    {
    //        ManTex->WaitForLoad();
    //        return ManTex;
    //    }
    //
    //    Utility::ByteArray ba = Utility::ReadFileSync(s_RootPath + fileName);
    //    if (ba->size() == 0 || !ManTex->CreateDDSFromMemory(ba->data(), ba->size(), sRGB))
    //        ManTex->SetToInvalidTexture();
    //    else
    //        ManTex->GetResource()->SetName(fileName.c_str());
    //
    //    return ManTex;
    //}
    //
    //const ManagedTexture* TextureManager::LoadTGAFromFile(const std::wstring& fileName, bool sRGB)
    //{
    //    auto ManagedTex = FindOrLoadTexture(fileName);
    //
    //    ManagedTexture* ManTex = ManagedTex.first;
    //    const bool RequestsLoad = ManagedTex.second;
    //
    //    if (!RequestsLoad)
    //    {
    //        ManTex->WaitForLoad();
    //        return ManTex;
    //    }
    //
    //    Utility::ByteArray ba = Utility::ReadFileSync(s_RootPath + fileName);
    //    if (ba->size() > 0)
    //    {
    //        ManTex->CreateTGAFromMemory(ba->data(), ba->size(), sRGB);
    //        ManTex->GetResource()->SetName(fileName.c_str());
    //    }
    //    else
    //        ManTex->SetToInvalidTexture();
    //
    //    return ManTex;
    //}
    //
    //
    //const ManagedTexture* TextureManager::LoadPIXImageFromFile(const std::wstring& fileName)
    //{
    //    auto ManagedTex = FindOrLoadTexture(fileName);
    //
    //    ManagedTexture* ManTex = ManagedTex.first;
    //    const bool RequestsLoad = ManagedTex.second;
    //
    //    if (!RequestsLoad)
    //    {
    //        ManTex->WaitForLoad();
    //        return ManTex;
    //    }
    //
    //    Utility::ByteArray ba = Utility::ReadFileSync(s_RootPath + fileName);
    //    if (ba->size() > 0)
    //    {
    //        ManTex->CreatePIXImageFromMemory(ba->data(), ba->size());
    //        ManTex->GetResource()->SetName(fileName.c_str());
    //    }
    //    else
    //        ManTex->SetToInvalidTexture();
    //
    //    return ManTex;
    //}

}
