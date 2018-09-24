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

#pragma once

#include "DX12GpuResource.h"
#include "Utility.h"

namespace Renderer 
{

    class DX12Texture : public DX12GpuResource
    {
    
    public:
    
        DX12Texture() { m_handle = {}; }
        DX12Texture(DescriptorHandle Handle) : m_handle(Handle) {}
    
        // Create a 1-level 2D texture
        void Create(std::wstring p_name,size_t Pitch, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData );
        void Create(std::wstring p_name,size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData )
        {
            Create(p_name,Width, Width, Height, Format, InitData);
        }
    
        void CreateTGAFromMemory( const void* memBuffer, size_t fileSize, bool sRGB );
        bool CreateDDSFromMemory( const void* memBuffer, size_t fileSize, bool sRGB );
        void CreatePIXImageFromMemory( const void* memBuffer, size_t fileSize );
    
        virtual void Destroy() override
        {
            DX12GpuResource::Destroy();
            m_handle.cpu_handle.ptr = 0;
            m_handle.gpu_handle.ptr = 0;

        }
    
        const DescriptorHandle& GetSRV() { return m_handle; };
        bool operator!() { return m_handle.cpu_handle.ptr == 0; }
    
    protected:
    
        DescriptorHandle m_handle;
    };
    
    class ManagedTexture : public DX12Texture
    {
    public:
        ManagedTexture( const std::wstring& FileName ) : m_MapKey(FileName), m_IsValid(true) {}
    
        void operator= ( const DX12Texture& Texture );
    
        void WaitForLoad(void) const;
        void Unload(void);
    
        void SetToInvalidTexture(void);
        bool IsValid(void) const { return m_IsValid; }
    
    private:
        std::wstring m_MapKey;		// For deleting from the map later
        bool m_IsValid;
    };
    
    namespace TextureManager
    {
        void Initialize( const std::wstring& TextureLibRoot );
        void Shutdown(void);
    
        const ManagedTexture* LoadFromFile( const std::wstring& fileName, bool sRGB = false );
        //const ManagedTexture* LoadDDSFromFile( const std::wstring& fileName, bool sRGB = false );
        //const ManagedTexture* LoadTGAFromFile( const std::wstring& fileName, bool sRGB = false );
        //const ManagedTexture* LoadPIXImageFromFile( const std::wstring& fileName );
    
        inline const ManagedTexture* LoadFromFile( const std::string& fileName, bool sRGB = false )
        {
            return LoadFromFile(MakeWStr(fileName), sRGB);
        }
    
        //inline const ManagedTexture* LoadDDSFromFile( const std::string& fileName, bool sRGB = false )
        //{
        //    return LoadDDSFromFile(MakeWStr(fileName), sRGB);
        //}
        //
        //inline const ManagedTexture* LoadTGAFromFile( const std::string& fileName, bool sRGB = false )
        //{
        //    return LoadTGAFromFile(MakeWStr(fileName), sRGB);
        //}
        //
        //inline const ManagedTexture* LoadPIXImageFromFile( const std::string& fileName )
        //{
        //    return LoadPIXImageFromFile(MakeWStr(fileName));
        //}
    
        const DX12Texture& GetBlackTex2D(void);
        const DX12Texture& GetWhiteTex2D(void);
    }
}

