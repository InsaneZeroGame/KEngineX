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


cbuffer MVP: register(b1)
{
    matrix proj;
    matrix view;
    matrix shadow_matrix;
    matrix model;

};




UIPSInput main(float3 position : POSITION, float4 color : COLOR,float2 texture_coord : TEXTURECOORD)
{
    UIPSInput result;
    matrix proj_view_matrix = mul(proj,view);
	result.position = mul(proj_view_matrix,float4(position,1.0f));
	result.color = color;
	return result;
}

