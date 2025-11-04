#include "Shared.hlsli"

PS_INPUT main(VS_INPUT_COMMON input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    float4x4 world = mul(g_bonePose[g_refBoneIndex], g_world);
    
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(output.pos, g_lightView);
    output.pos = mul(output.pos, g_lightProjection);
    
    output.tex = input.tex;
    
    return output;
}