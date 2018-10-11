#include "DX12Renderer.h"
#include "DXSampleHelper.h"
#include "AssetManager.h"
#include <future>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <IDataType.h>

Renderer::DX12Renderer::DX12Renderer():
    IRenderer(),
    m_viewport(0, 0.0f, static_cast<float>(m_window_width), static_cast<float>(m_window_height)),
    m_scissorRect(0, 0, static_cast<LONG>(m_window_width), static_cast<LONG>(m_window_height)),
    m_fence_value(),
    m_render_cmd(),
    m_device(DX12GpuDevice::GetGpuDevicePtr()),
    m_current_frameindex(m_device->GetSwapChain()->GetCurrentBackBufferIndex()),
    m_render_context(new DX12RenderContext)
{
}

void Renderer::DX12Renderer::Init()
{
    InitFences();
    InitCameraUniform();
    InitRootSignature();
    InitGraphicsPipelines();
    InitCmdBuffers();
}

void Renderer::DX12Renderer::InitFences()
{
    //Present Fence
    for (auto i = 0; i < DX12RendererConstants::SWAP_CHAIN_COUNT;++i)
    {
        m_device->GetDX12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&m_fences[i]));
    }
    //gpu fence value init to 0,so we set cpu side value to 1.
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
    D3D12_GRAPHICS_PIPELINE_STATE_DESC l_base_pso_desc = {};
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> shadow_map_vs;
        ComPtr<ID3DBlob> ui_vs;
        ComPtr<ID3DBlob> ui_ps;
        ComPtr<ID3DBlob> ui_ps_debug;
        ComPtr<ID3DBlob> ui_vs_debug;
        ComPtr<ID3DBlob> color_pass_gbuffer_ps;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"color_pass_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"color_pass_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags, 0, &pixelShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shadow_map_pass_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &shadow_map_vs, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ui_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &ui_vs, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ui_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags, 0, &ui_ps, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ui_vs_debug.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &ui_vs_debug, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"ui_ps_debug.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags, 0, &ui_ps_debug, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"color_pass_gbuffer_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags, 0, &color_pass_gbuffer_ps, nullptr));


        // Describe and create the graphics pipeline state object (PSO).
        l_base_pso_desc.InputLayout = { inputElementDescs.data(), static_cast<uint32_t>(inputElementDescs.size()) };
        l_base_pso_desc.pRootSignature = m_rootSignature.Get();
        l_base_pso_desc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        l_base_pso_desc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        l_base_pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        l_base_pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        l_base_pso_desc.DepthStencilState.DepthEnable = TRUE;
        l_base_pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        l_base_pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        l_base_pso_desc.DepthStencilState.StencilEnable = FALSE;
        l_base_pso_desc.SampleMask = UINT_MAX;
        l_base_pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        l_base_pso_desc.NumRenderTargets = 1;
        l_base_pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        l_base_pso_desc.DSVFormat = DEPTH_BUFFER_FORMAT;
        l_base_pso_desc.SampleDesc.Count = 1;


        //deferred Pass
        D3D12_GRAPHICS_PIPELINE_STATE_DESC deferred_pass = l_base_pso_desc;
        {
            //num of render targets = num of color attachment + swapchain;
            deferred_pass.NumRenderTargets = 1;
            deferred_pass.PS = CD3DX12_SHADER_BYTECODE(color_pass_gbuffer_ps.Get());
            for (auto i = 0 ; i < COLOR_ATTACHMENT_CONFIG.size(); ++i)
            {
                deferred_pass.NumRenderTargets++;
                deferred_pass.RTVFormats[i + 1] = COLOR_ATTACHMENT_CONFIG[i].Format;
            }

            ThrowIfFailed(m_device->GetDX12Device()->CreateGraphicsPipelineState(&deferred_pass, IID_PPV_ARGS(&m_deferred_pipelineState)));
        }

    
        //UI Pass
        D3D12_GRAPHICS_PIPELINE_STATE_DESC ui_pass_pso_desc = l_base_pso_desc;
        {
            ui_pass_pso_desc.VS = CD3DX12_SHADER_BYTECODE(ui_vs.Get());
            ui_pass_pso_desc.PS = CD3DX12_SHADER_BYTECODE(ui_ps.Get());
            ui_pass_pso_desc.DepthStencilState.DepthEnable = FALSE;
            ThrowIfFailed(m_device->GetDX12Device()->CreateGraphicsPipelineState(&ui_pass_pso_desc, IID_PPV_ARGS(&m_ui_pipelineState)));
        }


        //UI Debug pass (Line)
        D3D12_GRAPHICS_PIPELINE_STATE_DESC ui_debug_pass_pso_desc = l_base_pso_desc;
        {
            ui_debug_pass_pso_desc.VS = CD3DX12_SHADER_BYTECODE(ui_vs_debug.Get());
            ui_debug_pass_pso_desc.PS = CD3DX12_SHADER_BYTECODE(ui_ps_debug.Get());
            ui_debug_pass_pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            ThrowIfFailed(m_device->GetDX12Device()->CreateGraphicsPipelineState(&ui_debug_pass_pso_desc, IID_PPV_ARGS(&m_ui_debug_pipelineState)));
        }


        //Shadow Map Pass
        D3D12_GRAPHICS_PIPELINE_STATE_DESC shadow_pso_desc = l_base_pso_desc;
        {
            shadow_pso_desc.PS = {};
            shadow_pso_desc.VS = CD3DX12_SHADER_BYTECODE(shadow_map_vs.Get());
            shadow_pso_desc.RasterizerState.DepthBias = -10;
            shadow_pso_desc.RasterizerState.SlopeScaledDepthBias = -2.5f;
            shadow_pso_desc.RasterizerState.DepthBiasClamp = 0.0f;
            shadow_pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
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
    const auto swap_chain_frame = m_device->GetSwapChain()->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    //if (m_fences[swap_chain_frame]->GetCompletedValue() < m_fence_value[swap_chain_frame])
    //{
    //    ThrowIfFailed(m_fences[swap_chain_frame]->SetEventOnCompletion(m_fence_value[swap_chain_frame], m_fenceEvent));
    //    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    //}

    // Set the fence value for the next frame.
    m_fence_value[swap_chain_frame]++;

}

void Renderer::DX12Renderer::RecordGraphicsCmd()
{

    static bool is_first_frame = true;

    if (is_first_frame)
    {
        is_first_frame = false;
        DX12TransferManager::GetTransferManager().PrepareToRender();
    }

    //Shadow Map Pass
    {
        m_shadow_map_cmd->Reset();
        auto shadow_cmd = m_shadow_map_cmd->GetDX12CmdList();
        shadow_cmd->SetGraphicsRootSignature(m_rootSignature.Get());
        //Update Shadow Camera Uniform
        Math::Matrix4 mvp[2] = {
            m_scene->m_shadow_camera.GetProjMatrix(),
            m_scene->m_shadow_camera.GetViewMatrix()
        };
        memcpy(m_shadow_map_camera_uniform->data, mvp, sizeof(Math::Matrix4) * 2);
        shadow_cmd->SetGraphicsRootConstantBufferView(1, m_shadow_map_camera_uniform->GetGpuVirtualAddress());//+ CAMERA_UNIFORM_SIZE * m_current_frameindex);
        D3D12_VIEWPORT l_view_port = m_viewport;
        l_view_port.Width = DEPTH_BUFFER_WIDTH;
        l_view_port.Height = DEPTH_BUFFER_HEIGHT;
        shadow_cmd->RSSetViewports(1, &l_view_port);
        shadow_cmd->RSSetScissorRects(1, &m_scissorRect);
        shadow_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_render_context->PreShadowMapping(shadow_cmd);
        //Set Vertex and Index Buffer.
        SetVertexAndIndexBuffer(shadow_cmd);
        //Render Scene
        RenderScene(shadow_cmd);
        m_shadow_map_cmd->Flush();
        m_render_context->PostShadowMapping();
    }
    

    //Rendering Pass
    {
        m_render_cmd[m_current_frameindex]->Reset();
        auto current_render_cmd = m_render_cmd[m_current_frameindex]->GetDX12CmdList();

        current_render_cmd->SetGraphicsRootSignature(m_rootSignature.Get());
        //Update Main Camera uniform
        using namespace Math;
        auto shadow_sample_matrix = SHADOW_PREFIX * m_scene->m_shadow_camera.GetViewProjMatrix();
        Math::Matrix4 mvp[3] = { 
            m_scene->m_main_camera.GetProjMatrix(),
            m_scene->m_main_camera.GetViewMatrix(), 
            shadow_sample_matrix
        };
        memcpy(m_main_camera_uniform->data + CAMERA_UNIFORM_SIZE * m_current_frameindex, mvp, sizeof(Math::Matrix4) * 3);

        current_render_cmd->SetGraphicsRootConstantBufferView(1, m_main_camera_uniform->GetGpuVirtualAddress()+ CAMERA_UNIFORM_SIZE * m_current_frameindex);
        
        ID3D12DescriptorHeap* l_heaps[] = { m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get() };

        current_render_cmd->SetDescriptorHeaps(1, l_heaps);
        current_render_cmd->RSSetViewports(1, &m_viewport);
        current_render_cmd->RSSetScissorRects(1, &m_scissorRect);

        m_render_context->PreDeferredPass(current_render_cmd, m_current_frameindex);

        current_render_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        //Set Vertex and Index Buffer.
        SetVertexAndIndexBuffer(current_render_cmd);
        //Render Scene
        RenderScene(current_render_cmd);

        //Render UI(Scene Related Debug Content)
        current_render_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        current_render_cmd->SetPipelineState(m_ui_debug_pipelineState.Get());
        current_render_cmd->DrawIndexedInstanced(static_cast<uint32_t>(dummy_debug_ui->m_meshes[0]->GetIndexCount()), 1, static_cast<uint32_t>(dummy_debug_ui->m_meshes[0]->GetIndexOffsetInBuffer()), static_cast<int32_t>(dummy_debug_ui->m_meshes[0]->GetVertexOffsetInBuffer()), 0);


        //Render UI(Framebuffer Viewer)
        current_render_cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        current_render_cmd->SetPipelineState(m_ui_pipelineState.Get());
        current_render_cmd->DrawIndexedInstanced(static_cast<uint32_t>(dummy_depth_debug->m_meshes[0]->GetIndexCount()), 1, static_cast<uint32_t>(dummy_depth_debug->m_meshes[0]->GetIndexOffsetInBuffer()), static_cast<int32_t>(dummy_depth_debug->m_meshes[0]->GetVertexOffsetInBuffer()), 0);

        // Indicate that the back buffer will now be used to present.
        m_render_context->PrePresent(current_render_cmd, m_current_frameindex);
        ThrowIfFailed(current_render_cmd->Close());
    }
   
}

void Renderer::DX12Renderer::InitCmdBuffers()
{
    for (auto i = 0; i < m_render_cmd.size();++i)
    {
        m_render_cmd[i] = std::unique_ptr<DX12RenderCommndBuffer>(new DX12RenderCommndBuffer(m_deferred_pipelineState.Get()));
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
        //SRV Heap Layout
        //  Depth Buffer SRV
        //  ShadowMap SRV
        //  RTV 0 : Swapchain
        //  RTV 1 : Normal
        //  Material Textures.
        
        D3D12_ROOT_PARAMETER l_desc_table_parameter = {};
        l_desc_table_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        l_desc_table_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_DESCRIPTOR_RANGE shadow_map_desc_range;

        shadow_map_desc_range.BaseShaderRegister = 0;
        shadow_map_desc_range.NumDescriptors = 1;
        //Todo : Depth Buffer's SRV is the base desc handle in srv heap,shadow map is the second.Figure out a smart way to deal this.
        //Otherwise resources must be initialized in a fixed order.
        shadow_map_desc_range.OffsetInDescriptorsFromTableStart = 1;
        shadow_map_desc_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        shadow_map_desc_range.RegisterSpace = 0;

        D3D12_DESCRIPTOR_RANGE diffuse_desc_range;

        diffuse_desc_range.BaseShaderRegister = 1;
        //all srv except Shadow map and depth buffer
        diffuse_desc_range.NumDescriptors = DX12GpuDevice::DESCRIPTOR_HANDLE_MAX_NUM - 1;
        //Todo : Depth Buffer's SRV is the base desc handle in srv heap,shadow map is the second.Figure out a smart way to deal this.
        //Then it's color attachments(swapchain + other attachment).
        //Then it's textures
        const int l_swapchain_count = 1;
        diffuse_desc_range.OffsetInDescriptorsFromTableStart = 2 + COLOR_ATTACHMENT_CONFIG.size() + l_swapchain_count;
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
        l_default_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        l_default_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        l_default_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
        current_render_cmd->SetGraphicsRoot32BitConstants(0, 4, l_mesh->GetDiffuseMaterial(), 0);
        current_render_cmd->SetGraphicsRoot32BitConstant(2, l_mesh->GetTextureId(), 0);
        current_render_cmd->DrawIndexedInstanced(static_cast<uint32_t>(l_mesh->GetIndexCount()), 1, static_cast<uint32_t>(l_mesh->GetIndexOffsetInBuffer()), static_cast<int32_t>(l_mesh->GetVertexOffsetInBuffer()),0);
    }
}


void Renderer::DX12Renderer::SetVertexAndIndexBuffer(ID3D12GraphicsCommandList * p_cmd)
{   
    auto vertexbufferview = DX12TransferManager::GetTransferManager().GetVertexBufferView();
    p_cmd->IASetVertexBuffers(0, 1, &DX12TransferManager::GetTransferManager().GetVertexBufferView());
    auto bufferview = DX12TransferManager::GetTransferManager().GetIndexBufferView();
    p_cmd->IASetIndexBuffer(&DX12TransferManager::GetTransferManager().GetIndexBufferView());
}

void Renderer::DX12Renderer::SetTargetWindow(HWND hWnd, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
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

    //const int color_attachment_heap_offset = COLOR_ATTACHMENT_CONFIG.size();

    for (auto l_texture_id = 0  ; l_texture_id < m_scene->dummy_actor->m_texture_names.size() ; ++l_texture_id)
    {
        //Skip if default texture
        auto l_texture_name_to_load = m_scene->dummy_actor->m_texture_names[l_texture_id];
        if (l_texture_name_to_load == KEngineConstants::DEFAULT_TEXTURE_NAME) continue;
        //Skip if texture loaded already.
        int w, h, comp;
        unsigned char* image = stbi_load((l_texture_name_to_load).c_str(), &w, &h, &comp, STBI_rgb_alpha);
    
        uint8_t black_image[4] = { 0,0,0,0 };

        DX12Texture* l_texture = new DX12Texture;
        l_texture->Create(L"TextureFormDisk", 1, w, h, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, image);
        m_dummy_actor_textures.push_back(std::unique_ptr<DX12Texture>(l_texture));

        auto& meshes_use_this_texture = m_scene->dummy_actor->m_texture_map.equal_range(l_texture_name_to_load);
        for (auto l_mesh = meshes_use_this_texture.first; l_mesh != meshes_use_this_texture.second; ++l_mesh) {
            m_scene->dummy_actor->m_meshes[l_mesh->second]->SetTextureId(l_texture_id);
        }
    }

    //Init UI
    dummy_depth_debug = std::unique_ptr<gameplay::GameUIActor>(new gameplay::GameUIActor("Dummy UI"));
    auto l_ui_mesh = new gameplay::GameMesh("dummy_ui_mesh",Math::Rect(Math::Vector3(0.7f,1.0f,0.0f),0.3f));
    assetlib::AssetManager::GetAssertManager().LoadMesh(l_ui_mesh);
    dummy_depth_debug->AddMesh(l_ui_mesh);


    //Init Debug UI
    dummy_debug_ui = std::unique_ptr<gameplay::GameUIActor>(new gameplay::GameUIActor("Dummy Debug UI"));
    auto l_debug_ui_mesh = new gameplay::GameMesh("dummy_debug_ui_mesh", Math::Grid(15.0f, 30, {0.8f,0.8f,0.8f }));
    assetlib::AssetManager::GetAssertManager().LoadMesh(l_debug_ui_mesh);
    dummy_debug_ui->AddMesh(l_debug_ui_mesh);

    KFramework::IGPUStatic::ReleaseAllData();

}


void Renderer::DX12Renderer::Update()
{
    // Record all the commands we need to render the scene into the command list.
    RecordGraphicsCmd();

    // Execute the command list.
    std::vector<ID3D12CommandList*> ppCommandLists = { m_render_cmd[m_current_frameindex]->GetDX12CmdList() };
    m_device->GetCmdQueue()->ExecuteCommandLists(static_cast<uint32_t>(ppCommandLists.size()), ppCommandLists.data());

    // Present the frame.
    ThrowIfFailed(m_device->GetSwapChain()->Present(1, 0));

    WaitForPreviousFrame();
}

void Renderer::DX12Renderer::Destory()
{
    m_scene.reset();
}


Renderer::DX12Renderer::~DX12Renderer()
{

}
