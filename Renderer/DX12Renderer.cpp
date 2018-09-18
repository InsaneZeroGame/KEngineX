#include "DX12Renderer.h"
#include "DXSampleHelper.h"
#include <future>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Renderer::DX12Renderer::DX12Renderer():
    IRenderer(),
    m_viewport(0, 0.0f, static_cast<float>(m_window_width), static_cast<float>(m_window_height)),
    m_scissorRect(0, 0, static_cast<LONG>(m_window_width), static_cast<LONG>(m_window_height)),
    m_fence_value(),
    m_render_cmd(),
    m_device(DX12GpuDevice::GetGpuDevicePtr())
{
}

Renderer::DX12Renderer::~DX12Renderer()
{
}

void Renderer::DX12Renderer::Init()
{
    InitSwapChain();
    InitDepthBuffer();
    InitFences();
    InitCameraUniform();
    InitRootSignature();
    InitGraphicsPipelines();
    InitCmdBuffers();
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
    

    // Create the pipeline state, which includes compiling and loading shaders.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> shadow_map_vs;


#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"color_pass_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"color_pass_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags, 0, &pixelShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shadow_map_pass_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &shadow_map_vs, nullptr));

        // Define the vertex input layout.
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXTURECOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        };


        // Describe and create the graphics pipeline state object (PSO).
        psoDesc.InputLayout = { inputElementDescs.data(), static_cast<uint32_t>(inputElementDescs.size()) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DEPTH_BUFFER_FORMAT;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->GetDX12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    
        //Shadow Map Pass
        D3D12_GRAPHICS_PIPELINE_STATE_DESC shadow_pso_desc = psoDesc;
        {
            shadow_pso_desc.PS = {};
            shadow_pso_desc.VS = CD3DX12_SHADER_BYTECODE(shadow_map_vs.Get());
            shadow_pso_desc.RasterizerState.DepthBias = -10;
            shadow_pso_desc.RasterizerState.SlopeScaledDepthBias = -2.5f;
            shadow_pso_desc.RasterizerState.DepthBiasClamp = 0.0f;
            shadow_pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
            shadow_pso_desc.NumRenderTargets = 0;
            shadow_pso_desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
            shadow_pso_desc.BlendState.IndependentBlendEnable = FALSE;
            shadow_pso_desc.BlendState.RenderTarget[0].BlendEnable = FALSE;
            shadow_pso_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            shadow_pso_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            shadow_pso_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            shadow_pso_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            shadow_pso_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            shadow_pso_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            shadow_pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
            ThrowIfFailed(m_device->GetDX12Device()->CreateGraphicsPipelineState(&shadow_pso_desc, IID_PPV_ARGS(&m_shadow_map_pipelineState)));
        }
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
    //Shadow Map Pass
    {
        m_shadow_map_cmd->Reset();
        auto shadow_cmd = m_shadow_map_cmd->GetDX12CmdList();
        shadow_cmd->SetGraphicsRootSignature(m_rootSignature.Get());
        //Update Shadow Camera Uniform
        memcpy(m_shadow_map_camera_uniform->data + sizeof(float) * 16, &m_scene->m_shadow_camera.GetViewProjMatrix(), sizeof(float) * 16);
        shadow_cmd->SetGraphicsRootConstantBufferView(1, m_shadow_map_camera_uniform->GetGpuVirtualAddress());//+ CAMERA_UNIFORM_SIZE * m_current_frameindex);
        D3D12_VIEWPORT l_view_port = m_viewport;
        l_view_port.Width = DEPTH_BUFFER_WIDTH;
        l_view_port.Height = DEPTH_BUFFER_HEIGHT;
        shadow_cmd->RSSetViewports(1, &l_view_port);
        shadow_cmd->RSSetScissorRects(1, &m_scissorRect);
        shadow_cmd->ClearDepthStencilView(m_shadow_map->GetDSV().cpu_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
        shadow_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        shadow_cmd->OMSetRenderTargets(0, nullptr, FALSE, &m_shadow_map->GetDSV().cpu_handle);
        //Set Vertex and Index Buffer.
        SetVertexAndIndexBuffer(shadow_cmd);
        //Render Scene
        RenderScene(shadow_cmd);
        m_shadow_map_cmd->Flush();
        //Expose shadow map to shader.
        DX12TransferManager::GetTransferManager().TransitionResource(*m_shadow_map, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    

    //Rendering Pass
    {
        m_render_cmd[m_current_frameindex]->Reset();
        auto current_render_cmd = m_render_cmd[m_current_frameindex]->GetDX12CmdList();

        current_render_cmd->SetGraphicsRootSignature(m_rootSignature.Get());
        //Update Main Camera uniform
        using namespace Math;
        auto shadow_prefix = Math::Matrix4(AffineTransform(Matrix3::MakeScale(0.5f, -0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.0f)));
        auto shadow_sample_matrix = shadow_prefix * m_scene->m_shadow_camera.GetViewProjMatrix();
        memcpy(m_main_camera_uniform->data, &m_scene->m_main_camera.GetViewProjMatrix(), sizeof(float) * 16);
        memcpy(m_main_camera_uniform->data + sizeof(float) * 16, &shadow_sample_matrix, sizeof(float) * 16);

        current_render_cmd->SetGraphicsRootConstantBufferView(1, m_main_camera_uniform->GetGpuVirtualAddress());//+ CAMERA_UNIFORM_SIZE * m_current_frameindex);
        //To Get a gpu handle from a cpu one.

        ID3D12DescriptorHeap* l_heaps[] = { m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get() };

        current_render_cmd->SetDescriptorHeaps(1, l_heaps);
        current_render_cmd->SetGraphicsRootDescriptorTable(3, m_shadow_map->GetDepthSRV().gpu_handle);
        current_render_cmd->RSSetViewports(1, &m_viewport);
        current_render_cmd->RSSetScissorRects(1, &m_scissorRect);

        // Indicate that the back buffer will be used as a render target.
        current_render_cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_current_frameindex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_current_frameindex, m_rtvDescriptorSize);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        current_render_cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        current_render_cmd->ClearDepthStencilView(m_depth_buffer->GetDSV().cpu_handle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);
        current_render_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        current_render_cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_depth_buffer->GetDSV().cpu_handle);
        //Set Vertex and Index Buffer.
        SetVertexAndIndexBuffer(current_render_cmd);
        //Render Scene
        RenderScene(current_render_cmd);
        // Indicate that the back buffer will now be used to present.
        current_render_cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_current_frameindex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
        DX12TransferManager::GetTransferManager().TransitionResource(*m_shadow_map, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

        ThrowIfFailed(current_render_cmd->Close());
    }
   
}

void Renderer::DX12Renderer::InitCmdBuffers()
{
    for (auto i = 0; i < m_render_cmd.size();++i)
    {
        m_render_cmd[i] = std::unique_ptr<DX12RenderCommndBuffer>(new DX12RenderCommndBuffer(m_pipelineState.Get()));
    }

    m_shadow_map_cmd = std::unique_ptr<DX12RenderCommndBuffer>(new DX12RenderCommndBuffer(m_shadow_map_pipelineState.Get()));


}

void Renderer::DX12Renderer::InitCameraUniform()
{
    //Init A Uniform Buffer for MainCamera using triple-buffer shceme.
    //Matrix : 4x4 = 16 floats
    //MVP = Matrix * 3
    //Triple Buffer = MVP * 3;
    m_main_camera_uniform = std::unique_ptr<UniformBuffer>(new UniformBuffer(CAMERA_UNIFORM_SIZE * 3));

    m_shadow_map_camera_uniform = std::unique_ptr<UniformBuffer>(new UniformBuffer(CAMERA_UNIFORM_SIZE * 3));

}

void Renderer::DX12Renderer::InitDepthBuffer()
{
    m_shadow_map = std::unique_ptr<DX12DepthBuffer>(new DX12DepthBuffer());
    m_shadow_map->Create(L"ShadowMap", DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT, DEPTH_BUFFER_FORMAT);
    DX12TransferManager::GetTransferManager().TransitionResource(*m_shadow_map, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);


    //ToDo:May gain performance using DENY_SHADER_ACCESS.
    m_depth_buffer = std::unique_ptr<DX12DepthBuffer>(new DX12DepthBuffer());
    m_depth_buffer->Create(L"DepthBuffer", m_window_width, m_window_height, DEPTH_BUFFER_FORMAT);
    DX12TransferManager::GetTransferManager().TransitionResource(*m_depth_buffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, true, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Renderer::DX12Renderer::InitRootSignature()
{
    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;


        //Material Uniform (Constants)
        D3D12_ROOT_PARAMETER l_material_parameter = {};
        l_material_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        l_material_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        l_material_parameter.Constants.Num32BitValues = 4;
        l_material_parameter.Constants.RegisterSpace = 0;
        l_material_parameter.Constants.ShaderRegister = 0;

       
        //Camera Uniform (Constant Buffer View)
        D3D12_ROOT_PARAMETER l_camera_parameter = {};
        l_camera_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        l_camera_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        l_camera_parameter.Descriptor.RegisterSpace = 0;
        l_camera_parameter.Descriptor.ShaderRegister = 1;

        //Texture DescHeap Index.
        D3D12_ROOT_PARAMETER l_texture_id_parameter = {};
        l_texture_id_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        l_texture_id_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        l_texture_id_parameter.Constants.Num32BitValues = 1;
        l_texture_id_parameter.Constants.RegisterSpace = 0;
        l_texture_id_parameter.Constants.ShaderRegister = 2;


        


        //DesciptorTable
        D3D12_ROOT_PARAMETER l_desc_table_parameter = {};
        l_desc_table_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        l_desc_table_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_DESCRIPTOR_RANGE shadow_map_desc_range;

        shadow_map_desc_range.BaseShaderRegister = 0;
        shadow_map_desc_range.NumDescriptors = 1;
        shadow_map_desc_range.OffsetInDescriptorsFromTableStart = 0;
        shadow_map_desc_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        shadow_map_desc_range.RegisterSpace = 0;

        D3D12_DESCRIPTOR_RANGE diffuse_desc_range;

        diffuse_desc_range.BaseShaderRegister = 1;
        //all srv except Shadow map 
        diffuse_desc_range.NumDescriptors = DX12GpuDevice::DESCRIPTOR_HANDLE_MAX_NUM - 1;
        diffuse_desc_range.OffsetInDescriptorsFromTableStart = 0;
        diffuse_desc_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        diffuse_desc_range.RegisterSpace = 0;

        

        std::vector<D3D12_DESCRIPTOR_RANGE> ranges = 
        {
            shadow_map_desc_range,
            diffuse_desc_range
        };
        l_desc_table_parameter.DescriptorTable.NumDescriptorRanges = static_cast<uint32_t>(ranges.size());
        l_desc_table_parameter.DescriptorTable.pDescriptorRanges = ranges.data();


        std::vector<D3D12_ROOT_PARAMETER> l_parameters = {
            l_material_parameter,
            l_camera_parameter, 
            l_texture_id_parameter,
            l_desc_table_parameter,
        };

        D3D12_STATIC_SAMPLER_DESC l_shadow_sampler = {};
        l_shadow_sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        l_shadow_sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        l_shadow_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_shadow_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_shadow_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_shadow_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        l_shadow_sampler.RegisterSpace = 0;
        l_shadow_sampler.ShaderRegister = 0;


        D3D12_STATIC_SAMPLER_DESC l_default_sampler = {};
        l_default_sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        l_default_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_default_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_default_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_default_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        l_default_sampler.RegisterSpace = 0;
        l_default_sampler.ShaderRegister = 1;


        std::vector<D3D12_STATIC_SAMPLER_DESC> l_static_samplers = 
        {
            l_shadow_sampler,
            l_default_sampler
        };

        rootSignatureDesc.Init(static_cast<uint32_t>(l_parameters.size()), l_parameters.data(), static_cast<uint32_t>(l_static_samplers.size()),l_static_samplers.data());
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error));
        ThrowIfFailed(m_device->GetDX12Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }
}

void Renderer::DX12Renderer::RenderScene(ID3D12GraphicsCommandList* current_render_cmd)
{
    for (auto & l_mesh : m_scene->dummy_actor->m_meshes)
    {
        current_render_cmd->SetGraphicsRoot32BitConstants(0, 4, l_mesh->m_diffuse.data(), 0);
        //int texture_id = m_dummy_actor_textures[m_scene->dummy_actor->m_mesh->m_texture_names[submesh.m_texture_id]]->m_descriptor_heap_index;
        current_render_cmd->SetGraphicsRoot32BitConstant(2, 0, 0);
        current_render_cmd->DrawIndexedInstanced(static_cast<uint32_t>(l_mesh->m_index_count), 1, static_cast<uint32_t>(l_mesh->m_index_offset), static_cast<int32_t>(l_mesh->m_vertex_offset),0);
    }
}

void Renderer::DX12Renderer::SetVertexAndIndexBuffer(ID3D12GraphicsCommandList * p_cmd)
{   
    auto vertexbufferview = DX12TransferManager::GetTransferManager().GetVertexBufferView();
    p_cmd->IASetVertexBuffers(0, 1, &DX12TransferManager::GetTransferManager().GetVertexBufferView());
    auto bufferview = DX12TransferManager::GetTransferManager().GetIndexBufferView();
    p_cmd->IASetIndexBuffer(&DX12TransferManager::GetTransferManager().GetIndexBufferView());
}

void Renderer::DX12Renderer::SetWindow(HWND hWnd, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    m_hwnd = hWnd;
    m_window_height = height;
    m_window_width = width;
    m_window_x_offset = x;
    m_window_y_offset = y;
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_scissorRect.right = width;
    m_scissorRect.bottom = height;
    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
}
void Renderer::DX12Renderer::SetCurrentScene(std::shared_ptr<gameplay::GamesScene> p_scene)
{
    if (m_scene)
    {
        //Release Previouse scene.
        m_scene.reset();
    }
    m_scene = p_scene;

    for (auto & l_mesh : m_scene->dummy_actor->m_meshes)
    {
        for (auto & l_texture_name_to_load : l_mesh->m_texture_names)
        {
            //Skip if default texture
            if (l_texture_name_to_load == KEngineConstants::DEFAULT_TEXTURE_NAME) continue;
            //Skip if texture loaded already.
            if (m_dummy_actor_textures.find(l_texture_name_to_load) != m_dummy_actor_textures.end()) continue;
            int w, h, comp;
            unsigned char* image = stbi_load((KEngineConstants::ASSET_DIR + l_texture_name_to_load).c_str(), &w, &h, &comp, STBI_rgb_alpha);

            uint8_t black_image[4] = { 0,0,0,0 };

            //If texture not found,leave it as 1x1 black image.
            if (!image)
            {
                DX12Texture* l_texture = new DX12Texture;
                l_texture->Create(1, 1, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, black_image);
                m_dummy_actor_textures.insert(std::pair<std::string, std::unique_ptr<DX12Texture>>(l_texture_name_to_load, std::unique_ptr<DX12Texture>(l_texture)));
            }
            else
            {
                DX12Texture* l_texture = new DX12Texture;
                l_texture->Create(1, w, h, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, image);
                m_dummy_actor_textures.insert(std::pair<std::string, std::unique_ptr<DX12Texture>>(l_texture_name_to_load, std::unique_ptr<DX12Texture>(l_texture)));

            }

        }

    }



}


void Renderer::DX12Renderer::Update()
{
    // Record all the commands we need to render the scene into the command list.
    RecordGraphicsCmd();

    // Execute the command list.
    std::vector<ID3D12CommandList*> ppCommandLists = { m_render_cmd[m_current_frameindex]->GetDX12CmdList() };
    m_device->GetCmdQueue()->ExecuteCommandLists(static_cast<uint32_t>(ppCommandLists.size()), ppCommandLists.data());

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void Renderer::DX12Renderer::Destory()
{
    m_scene.reset();
}
