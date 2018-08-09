#pragma once
#include <INoCopy.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <wrl.h>
#include "DX12RendererConstants.h"

namespace Renderer
{
    class DX12GpuDevice : public KFramework::INoCopy
    {
    public:
        __forceinline static DX12GpuDevice& GetGpuDevice() 
        {
            static DX12GpuDevice l_device;
            return l_device;
        }

        __forceinline static DX12GpuDevice* GetGpuDevicePtr()
        {
            return &GetGpuDevice();
        }

        __forceinline ID3D12CommandQueue* GetCmdQueue()
        {
            return m_commandQueue.Get();
        }

        __forceinline ID3D12Device* GetDevice()
        {
            return m_device.Get();
        }

        __forceinline ID3D12CommandAllocator* GetCmdAllcoator(uint8_t p_index)
        {
            return m_commandAllocator[p_index].Get();
        }

        bool CreateGraphicsCmdList(
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> p_cmdList,
            Microsoft::WRL::ComPtr<ID3D12PipelineState> p_pipeline_state,
            D3D12_COMMAND_LIST_TYPE p_type);

        ~DX12GpuDevice();
    private:

        DX12GpuDevice();

    private:
        bool m_useWarpDevice;

        void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

        Microsoft::WRL::ComPtr<IDXGIAdapter3> m_adapter;

        Microsoft::WRL::ComPtr<ID3D12Device> m_device;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[3];




    };//DX12GpuDevice
}//Renderer