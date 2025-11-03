#include "Shared.hlsli"

PS_INPUT main(VS_INPUT_COMMON input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    float4x4 world = mul(g_bonePose[g_refBoneIndex], g_world);
    
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = output.pos.xyz;
    output.pos = mul(output.pos, g_view);
    output.pos = mul(output.pos, g_projection);
    
    output.norm = mul(input.norm, (float3x3) world);
    output.tan = mul(input.tan, (float3x3) world);
    output.binorm = mul(input.binorm, (float3x3) world);
    
    output.tex = input.tex;
    
    return output;
}