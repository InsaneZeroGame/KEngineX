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



PSInput main(float3 position : POSITION, float4 color : COLOR)
{
    PSInput shadow_map_out;
    shadow_map_out.position = mul(modelToShadow, float4(position,1.0));
    return shadow_map_out;
}