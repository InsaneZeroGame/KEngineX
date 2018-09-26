#include "common.hlsli"




UIPSInput main(float3 pos : POSITION,float4 color : COLOR,float2 texture_coord : TEXTURECOORD)
{
    UIPSInput res;
    res.position = float4(pos,1.0f);
    res.color = color;
    res.texture_coord = texture_coord;
}