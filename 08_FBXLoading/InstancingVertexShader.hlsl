#include "Shared.hlsli"

PS_INPUT main(INS_VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    float4x4 insWorld = float4x4(input.world0, input.world1, input.world2, input.world3);
    
    output.pos = mul(float4(input.pos, 1.0f), insWorld);
    output.worldPos = output.pos.xyz;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    output.norm = mul(input.norm, (float3x3) insWorld);
    output.tan = mul(input.tan, (float3x3) insWorld);
    output.binorm = mul(input.binorm, (float3x3) insWorld);
    
    output.tex = input.tex;
    
    return output;
}