#include "Shared.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = output.pos.xyz;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    output.norm = mul(input.norm, (float3x3) normalMatrix);
    output.tan = mul(input.tan, (float3x3) normalMatrix);
    output.binorm = mul(input.binorm, (float3x3) normalMatrix);
    
    output.tex = input.tex;
    
    return output;
}