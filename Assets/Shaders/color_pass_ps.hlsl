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



SamplerComparisonState shadow_sampler : register(s0);

Texture2D<float> shadow_map : register(t0);

float4 main(PSInput input) : SV_TARGET
{
    float shadow = shadow_map.SampleCmpLevelZero(shadow_sampler,input.shadow_coord.xy,input.shadow_coord.z);

    return shadow * input.color;
}
