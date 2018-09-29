#include "common.hlsli"

SamplerState DefaultSampler : register(s1);
Texture2D shadow_map : register(t0);
Texture2D m_diffuse[49] : register(t1);


float4 main(UIPSInput input) : SV_TARGET
{
    float shadow_r = shadow_map.Sample(DefaultSampler,input.texture_coord).x;
    return float4(shadow_r, shadow_r, shadow_r,1.0f);
}