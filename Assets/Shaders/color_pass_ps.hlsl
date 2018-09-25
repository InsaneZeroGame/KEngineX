//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "common.hlsli"




#define PCF_SAMPLE_NUM 16;
#pragma warning (disable: 3078)

cbuffer myTextureID : register(b2)
{
    int myTextureId;
};


SamplerComparisonState shadow_sampler : register(s0);
SamplerState default_sampler : register(s1);

Texture2D<float> shadow_map : register(t0);
Texture2D mesh_diffuse[49] : register(t1);

float pcf(float3 shadow_coord)
{

    float2 poissonDisk[16] = {
        float2(-0.94201624 / 1920.0f, -0.39906216 / 1080.0f),
        float2(0.94558609  / 1920.0f, -0.76890725 / 1080.0f),
        float2(-0.094184101/ 1920.0f, -0.92938870 / 1080.0f),
        float2(0.34495938  / 1920.0f, 0.29387760  / 1080.0f),
        float2(-0.91588581 / 1920.0f, 0.45771432  / 1080.0f),
        float2(-0.81544232 / 1920.0f, -0.87912464 / 1080.0f),
        float2(-0.38277543 / 1920.0f, 0.27676845  / 1080.0f),
        float2(0.97484398  / 1920.0f, 0.75648379  / 1080.0f),
        float2(0.44323325  / 1920.0f, -0.97511554 / 1080.0f),
        float2(0.53742981  / 1920.0f, -0.47373420 / 1080.0f),
        float2(-0.26496911 / 1920.0f, -0.41893023 / 1080.0f),
        float2(0.79197514  / 1920.0f, 0.19090188  / 1080.0f),
        float2(-0.24188840 / 1920.0f, 0.99706507  / 1080.0f),
        float2(-0.81409955 / 1920.0f, 0.91437590  / 1080.0f),
        float2(0.19984126  / 1920.0f, 0.78641367  / 1080.0f),
        float2(0.14383161  / 1920.0f, -0.14100790 / 1080.0f)
    };

    float shadow = 0.0f;
    for (int i = 0; i < 16; ++i){
        shadow += shadow_map.SampleCmpLevelZero(shadow_sampler, shadow_coord.xy + poissonDisk[i], shadow_coord.z);
    }
    return shadow / 16;
}


//float pcf(float3 shadow_coord)
//{
//    float shadow = 0.0f;
//
//    for (int u = -2; u < 3;++u) 
//    {
//        for (int v = -2; v < 3; ++v)
//        {
//            float2 offset = float2(float(u) / 800.0, float(v) / 600.0);
//            shadow += shadow_map.SampleCmpLevelZero(shadow_sampler, shadow_coord.xy + offset, shadow_coord.z);
//        }
//    }
//    return shadow / 25;
//}





float4 main(PSInput input) : SV_TARGET
{
    //float shadow = shadow_map.SampleCmpLevelZero(shadow_sampler,input.shadow_coord.xy,input.shadow_coord.z);
    float shadow = pcf(input.shadow_coord);
    float4 diffuse = mesh_diffuse[myTextureId].Sample(default_sampler, input.texture_coord);
    return diffuse;
    //return (diffuse * 0.75) + float4(0.25, 0.25, 0.25, 0.25) * input.color;
    //return input.color;
}
