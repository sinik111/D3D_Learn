#include "Shared.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    
    float4x4 worldView = mul(g_world, g_view);
    float4x4 worldViewProjection = mul(worldView, g_projection);
    
    output.position = mul(float4(input.position, 1.0f), worldViewProjection);
    
    output.viewPos = mul(float4(input.position, 1.0f), worldView).xyz;
    
    output.viewLightDir = normalize(mul(-g_lightDirection, (float3x3) g_view));
    
    output.normal = normalize(mul(input.normal, (float3x3) worldView));
    
    output.texCoord = input.texCoord;
    
    output.localPos = input.position;
    
    return output;
}