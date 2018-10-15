#pragma once
#include <string>
#include <dxgi.h>
#include <d3d12.h>
#include <vector>
#include <EngineConfig.h>


namespace Renderer
{
    struct ColorAttachmentDescriptor
    {
        const std::wstring Name;
        uint32_t Width;
        uint32_t Height;
        DXGI_FORMAT Format;
        uint32_t NumMips;
        D3D12_GPU_VIRTUAL_ADDRESS VidMem;
    };


    //color attachments config used in pipelinestate renderpass and render context.
    const std::vector<ColorAttachmentDescriptor> COLOR_ATTACHMENT_CONFIG = 
    {
        {L"NormalAttachment",KEngineConstants::WINDOW_WIDTH,KEngineConstants::WINDOW_HEIGHT,DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,1,0}
    };


    // Define the vertex input layout.
    const std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURECOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

    };

    const int DEPTH_BUFFER_WIDTH = 1920;
    const int DEPTH_BUFFER_HEIGHT = 1080;


    const bool ENABLE_ACTOR_BOUNDING_BOX = true;
}