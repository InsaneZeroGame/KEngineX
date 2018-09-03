#pragma once
#include "DX12GpuDevice.h"
#include <INoCopy.h>


namespace Renderer
{
    class DX12CommandBuffer : public KFramework::INoCopy
    {
    public:
        DX12CommandBuffer(D3D12_COMMAND_LIST_TYPE p_type) :
            m_device(DX12GpuDevice::GetGpuDevicePtr()),
            m_cmd_allocator(nullptr)
        {
            m_device->GetDX12Device()->CreateCommandAllocator(p_type, IID_PPV_ARGS(&m_cmd_allocator));
        }
        virtual ~DX12CommandBuffer()
        {
            
        }

        virtual void Reset() const{};

        virtual void Flush() const{};

        virtual void Close() const {};

    protected:
        DX12GpuDevice * m_device;
        ID3D12CommandAllocator* m_cmd_allocator;
    };


    class DX12RenderCommndBuffer final : public DX12CommandBuffer
    {
    public:
        DX12RenderCommndBuffer(ID3D12PipelineState* p_state):
            DX12CommandBuffer(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT),
            m_cmd_buffer(nullptr),
            m_state(p_state)
        {
            m_device->GetDX12Device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator, p_state, IID_PPV_ARGS(&m_cmd_buffer));
            m_cmd_buffer->Close();
        }
        ~DX12RenderCommndBuffer()
        {

        }

        virtual void Reset() const 
        {
            m_cmd_allocator->Reset();
            m_cmd_buffer->Reset(m_cmd_allocator, m_state);
        }

        virtual void Flush() const 
        {
            Close();
            ID3D12CommandList* l_cmd_lists[] = {m_cmd_buffer};
            m_device->FlushCmd(l_cmd_lists,1);

        }
        virtual void Close() const
        {
            m_cmd_buffer->Close();
        }

        __forceinline  ID3D12GraphicsCommandList* GetDX12CmdList()
        {
            return m_cmd_buffer;
        }

    private:
        ID3D12GraphicsCommandList * m_cmd_buffer;

        ID3D12PipelineState* m_state;
    };
}

