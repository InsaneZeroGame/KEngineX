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


cbuffer myMaterial: register(b0)
{
    float4 diffuse;
};

cbuffer MVP: register(b1)
{
    matrix proj;
    matrix view;
    matrix shadow_matrix;
    matrix model;

};




PSInput main(float3 position : POSITION, float4 color : COLOR,float2 texture_coord : TEXTURECOORD)
{
	PSInput result;
    matrix proj_view_matrix = mul(proj,view);
    matrix proj_view_model_matrix = mul(proj_view_matrix, model);
	result.position = mul(proj_view_model_matrix,float4(position,1.0f));
	result.color = color;
    matrix shadow_model_matrix = mul(shadow_matrix, model);
    float4 shadow_coord = mul(shadow_model_matrix, float4(position, 1.0f));
    result.shadow_coord = shadow_coord.xyz / shadow_coord.w;
    result.texture_coord = texture_coord;
	return result;
}

