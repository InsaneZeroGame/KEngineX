#include "DX12Renderer.h"
#include "DXSampleHelper.h"

Renderer::DX12Renderer::DX12Renderer():
    m_hwnd(nullptr),
    m_device(nullptr)
{
}

Renderer::DX12Renderer::~DX12Renderer()
{
}

void Renderer::DX12Renderer::InitDevice()
{
    m_device = DX12GpuDevice::GetGpuDevicePtr();
}

void Renderer::DX12Renderer::Init()
{
    InitDevice();
}

void Renderer::DX12Renderer::Update()
{
}

void Renderer::DX12Renderer::Destory()
{
}
