#pragma once
#include <INoCopy.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

namespace Renderer
{
    class DX12GpuDevice : public KFramework::INoCopy
    {
    public:
        static DX12GpuDevice& GetGpuDevice() 
        {
            static DX12GpuDevice l_device;
            return l_device;
        }
        ~DX12GpuDevice();
    private:
        DX12GpuDevice();
    };//DX12GpuDevice
}//Renderer