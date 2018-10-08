#pragma once
#include <INoCopy.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <wrl.h>
#include "DX12RendererConstants.h"
#include "Utility.h"
#include <array>

namespace Renderer
{

    struct DescriptorHandle
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
    };



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

        __forceinline ID3D12Device* GetDX12Device()
        {
            return m_device.Get();
        }

        void FlushCmd(ID3D12CommandList** p_cmd,uint32_t count);

        
        DescriptorHandle GetDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE p_type,uint32_t* index = nullptr);
       
        __forceinline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE p_type)
        {
            return m_desc_heaps[p_type];
        }

        __forceinline Microsoft::WRL::ComPtr<IDXGISwapChain3> GetSwapChain()
        {
            return m_swapChain;
        }

        enum { DESCRIPTOR_HANDLE_MAX_NUM = 50 };


        ~DX12GpuDevice();
    private:

        DX12GpuDevice();

    private:

        enum { DESCRIPTOR_TYPE_NUM = 5 };
        enum { FENCE_WAIT_TIME_INFINITY = 0xffffffffffffffff};


        bool m_useWarpDevice;

        void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

        Microsoft::WRL::ComPtr<IDXGIAdapter3> m_adapter;

        Microsoft::WRL::ComPtr<ID3D12Device> m_device;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;

        void AllocateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE);

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_desc_heaps[DESCRIPTOR_TYPE_NUM];

        std::array<uint32_t, DESCRIPTOR_TYPE_NUM> m_desc_heaps_handle_count;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_cmd_flush_fence;

        uint64_t m_cmd_flush_fence_value = 1;

        HANDLE m_cmd_flush_wait_event = nullptr;

        void WaitForGPU();

    };//DX12GpuDevice
}//Renderer