#include "common.hlsli"


cbuffer myMaterial: register(b0)
{
    float4 diffuse;
};

cbuffer MVP: register(b1)
{
    matrix proj;
    matrix view;
};

cbuffer ActorBuffer: register(b3)
{
    matrix model_matrix;
};



PSInput main(float3 position : POSITION, float4 color : COLOR,float2 texture_coord : TEXTURECOORD)
{
    PSInput shadow_map_out;
    matrix proj_view_matrix = mul(proj,view);
    matrix proj_view_model_matrix = mul(proj_view_matrix, model_matrix);
    shadow_map_out.position = mul(proj_view_model_matrix, float4(position,1.0));
    return shadow_map_out;
}