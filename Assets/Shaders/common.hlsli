




struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 shadow_coord : TexCoord0;
    float2 texture_coord : TexCoord1;
};


struct UIPSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texture_coord : TexCoord0;
};