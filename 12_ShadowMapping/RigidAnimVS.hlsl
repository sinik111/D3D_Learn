#include "Shared.hlsli"

PS_INPUT_SHADOW main(VS_INPUT_COMMON input)
{
    PS_INPUT_SHADOW output = (PS_INPUT_SHADOW) 0;
    
    float4x4 world = mul(g_bonePose[g_refBoneIndex], g_world);
    
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = output.pos.xyz;
    output.pos = mul(output.pos, g_view);
    output.pos = mul(output.pos, g_projection);
    
    output.norm = mul(input.norm, (float3x3) world);
    output.tan = mul(input.tan, (float3x3) world);
    output.binorm = mul(input.binorm, (float3x3) world);
    
    output.tex = input.tex;
    
    output.lightViewPos = mul(float4(output.worldPos, 1.0f), g_lightView);
    output.lightViewPos = mul(output.lightViewPos, g_lightProjection);
    
    return output;
}