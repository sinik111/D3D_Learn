#include "Shared.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;
    
    output.pos = worldPos;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    output.norm = normalize(mul(input.norm, (float3x3) normalMatrix));
    
    output.tex = input.tex;
    
    return output;
}