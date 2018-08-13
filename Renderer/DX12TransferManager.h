#pragma once
#include "DX12GpuDevice.h"
#include <INoCopy.h>



namespace Renderer 
{
    class DX12TransferManager final : public KFramework::INoCopy
    {
    public:

        ~DX12TransferManager();

        __forceinline static DX12TransferManager& GetGpuDevice()
        {
            static DX12TransferManager l_device;
            return l_device;
        }

        __forceinline static DX12TransferManager* GetGpuDevicePtr()
        {
            return &GetGpuDevice();
        }

    private:
        DX12TransferManager();
        DX12GpuDevice* m_device;

    };//DX12UploadManager
}//Renderer