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
    matrix modelToProj;
    matrix modelToShadow;
    matrix project;

};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;

	result.position = mul(modelToProj,position);
	result.color = diffuse;

	return result;
}

