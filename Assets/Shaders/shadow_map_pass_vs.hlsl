#include "common.hlsli"


cbuffer myMaterial: register(b0)
{
    float4 diffuse;
};

cbuffer MVP: register(b1)
{
    matrix proj;
    matrix view;
    matrix model;

};



PSInput main(float3 position : POSITION, float4 color : COLOR,float2 texture_coord : TEXTURECOORD)
{
    PSInput shadow_map_out;
    matrix proj_view_matrix = mul(proj,view);
    shadow_map_out.position = mul(proj_view_matrix, float4(position,1.0));
    return shadow_map_out;
}