#include "DX12Renderer.h"
#include "DXSampleHelper.h"
#include "AssetManager.h"

Renderer::DX12Renderer::DX12Renderer():
    IRenderer(),
    m_device(nullptr),
    m_viewport(0.0f, 0.0f, static_cast<float>(m_window_width), static_cast<float>(m_window_height)),
    m_scissorRect(0, 0, static_cast<LONG>(m_window_width), static_cast<LONG>(m_window_height)),
    m_fence_value(),
    m_render_cmd()
{
}

Renderer::DX12Renderer::~DX12Renderer()
{
}

void Renderer::DX12Renderer::Init()
{
    InitDevice();
    auto& manager = DX12TransferManager::GetTransferManager();
    InitSwapChain();
    InitFences();
    InitGraphicsPipelines();
    InitCmdBuffers();
}


void Renderer::DX12Renderer::InitDevice()
{
    m_device = DX12GpuDevice::GetGpuDevicePtr();
}

void Renderer::DX12Renderer::InitSwapChain()
{
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = DX12RendererConstants::SWAP_CHAIN_COUNT;
    swapChainDesc.Width = m_window_width;
    swapChainDesc.Height = m_window_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_device->GetCmdQueue(),		// Swap chain needs the queue so that it can force a flush on it.
        m_hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_current_frameindex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = DX12RendererConstants::SWAP_CHAIN_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->GetDX12Device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDX12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < DX12RendererConstants::SWAP_CHAIN_COUNT; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->GetDX12Device()->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

}

void Renderer::DX12Renderer::InitFences()
{
    //Present Fence
    for (auto i = 0; i < DX12RendererConstants::SWAP_CHAIN_COUNT;++i)
    {
        m_device->GetDX12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&m_fences[i]));
    }
    m_fence_value[0] = 1;

    // Create an event handle to use for frame synchronization.
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
}

void Renderer::DX12Renderer::InitGraphicsPipelines()
{
    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->GetDX12Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->GetDX12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

}

void Renderer::DX12Renderer::LoadScene(std::shared_ptr<gameplay::GamesScene> p_scene)
{

    assetlib::AssetManager::GetAssertManager();

    assert(p_scene && "Nullptr!");
    m_scene = p_scene;
    {
        struct Vertex
        {
            float position[3];
            float color[4];
        };
        // Define the geometry for a triangle.

        for (auto& material : m_scene->dummy_actor->m_meterial)
        {
            for (auto& mesh : material->m_meshes)
            {
                const UINT vertexBufferSize = mesh.m_vertices.size() * sizeof(float);


                TransferJob l_vertex_upload_job = {};
                l_vertex_upload_job.data = mesh.m_vertices.data();
                l_vertex_upload_job.data_size = vertexBufferSize;
                l_vertex_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX_BUFFER;

                DX12TransferManager::GetTransferManager().AddJob(&l_vertex_upload_job, true);
                // Initialize the vertex buffer view.
                mesh.m_vertex_buffer_desc.BufferLocation = l_vertex_upload_job.gpu_va_address;
                mesh.m_vertex_buffer_desc.StrideInBytes = sizeof(Vertex);
                mesh.m_vertex_buffer_desc.SizeInBytes = vertexBufferSize;

                TransferJob l_submesh_upload_job = {};
                l_submesh_upload_job.data = mesh.m_indices.data();
                l_submesh_upload_job.data_size = sizeof(uint32_t) * mesh.m_indices.size();
                l_submesh_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX_BUFFER;

                DX12TransferManager::GetTransferManager().AddJob(&l_submesh_upload_job, true);
                // Initialize the Index buffer view.
                mesh.m_index_buffer_desc.BufferLocation = l_submesh_upload_job.gpu_va_address;
                mesh.m_index_buffer_desc.StrideInBytes = sizeof(uint32_t);
                mesh.m_index_buffer_desc.SizeInBytes = l_submesh_upload_job.data_size;
            }
        }

        DX12TransferManager::GetTransferManager().PrepareToRender();

        
        //WaitForPreviousFrame();
    }
}

void Renderer::DX12Renderer::WaitForPreviousFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fence_value[m_current_frameindex];
    ThrowIfFailed(m_device->GetCmdQueue()->Signal(m_fences[m_current_frameindex], currentFenceValue));
    m_current_frameindex = (m_current_frameindex + 1) % 3;
    // Update the frame index.
    const auto swap_chain_frame = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fences[swap_chain_frame]->GetCompletedValue() < m_fence_value[swap_chain_frame])
    {
        ThrowIfFailed(m_fences[swap_chain_frame]->SetEventOnCompletion(m_fence_value[swap_chain_frame], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fence_value[swap_chain_frame]++;

}

void Renderer::DX12Renderer::RecordGraphicsCmd()
{

    // Set necessary state.
    m_render_cmd[m_current_frameindex]->Reset();
    auto current_render_cmd = m_render_cmd[m_current_frameindex]->GetDX12CmdList();
    current_render_cmd->SetGraphicsRootSignature(m_rootSignature.Get());
    current_render_cmd->RSSetViewports(1, &m_viewport);
    current_render_cmd->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    current_render_cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_current_frameindex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_current_frameindex, m_rtvDescriptorSize);
    current_render_cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    current_render_cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    current_render_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (auto& material : m_scene->dummy_actor->m_meterial)
    {
        for (auto& mesh : material->m_meshes)
        {
            D3D12_VERTEX_BUFFER_VIEW l_mesh_desc = {};
            l_mesh_desc.BufferLocation = mesh.m_vertex_buffer_desc.BufferLocation;
            l_mesh_desc.SizeInBytes = mesh.m_vertex_buffer_desc.SizeInBytes;
            l_mesh_desc.StrideInBytes = mesh.m_vertex_buffer_desc.StrideInBytes;


            D3D12_INDEX_BUFFER_VIEW l_sub_mesh_desc = {};
            l_sub_mesh_desc.BufferLocation = mesh.m_index_buffer_desc.BufferLocation;
            l_sub_mesh_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
            l_sub_mesh_desc.SizeInBytes = mesh.m_index_buffer_desc.SizeInBytes;
            current_render_cmd->IASetIndexBuffer(&l_sub_mesh_desc);
            current_render_cmd->IASetVertexBuffers(0, 1, &l_mesh_desc);
            current_render_cmd->DrawIndexedInstanced(mesh.m_index_count, 1, 0, 0, 0);
        }
    }
    

    // Indicate that the back buffer will now be used to present.
    current_render_cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_current_frameindex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(current_render_cmd->Close());
}

void Renderer::DX12Renderer::InitCmdBuffers()
{
    for (auto i = 0; i < m_render_cmd.size();++i)
    {
        m_render_cmd[i] = new DX12RenderCommndBuffer(m_pipelineState.Get());
    }
}

void Renderer::DX12Renderer::SetWindow(HWND hWnd, uint32_t width, uint32_t height)
{
    m_hwnd = hWnd;
    m_window_height = height;
    m_window_width = width;
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_scissorRect.right = width;
    m_scissorRect.bottom = height;
};



void Renderer::DX12Renderer::Update()
{
    // Record all the commands we need to render the scene into the command list.
    RecordGraphicsCmd();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_render_cmd[m_current_frameindex]->GetDX12CmdList() };
    m_device->GetCmdQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void Renderer::DX12Renderer::Destory()
{
    for (auto cmd : m_render_cmd) {
        if (cmd)
        {
            delete cmd;
            cmd = nullptr;
        }
    }
}
