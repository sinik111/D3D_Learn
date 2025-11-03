#include "Shared.hlsli"

PS_INPUT_OUTLINE main(VS_INPUT_OUTLINE input)
{
    PS_INPUT_OUTLINE output;
    
    float3 N = normalize(input.normal);
    
    const float2 bounds = float2(90.0f, 40.0f);
    
    float x = (input.position.x + bounds.x) / bounds.x * 0.5f;
    float y = (input.position.y + bounds.y) / bounds.y * 0.5f;
    
    float spatialPhaseOffset = (x + y) * g_outlineDensity;
    
    float timePhase = g_elapsedTime * g_outlineFrequency;
    
    float totalPhase = timePhase + spatialPhaseOffset;
    
    float r = (sin(totalPhase) + 1.0f) * 0.5f;
    float g = (sin(totalPhase - 2.0f) + 1.0f) * 0.5f;
    float b = (sin(totalPhase - 4.0f) + 1.0f) * 0.5f;
    
    float dynamicThickness = (r + 0.5f) * g_outlineThickness;
    
    float3 expandedPos = input.position + (N * dynamicThickness);
    
    output.position = mul(float4(expandedPos, 1.0f), g_world);
    output.position = mul(output.position, g_view);
    output.position = mul(output.position, g_projection);
    
    output.color = float4(r, g, b, 1.0f);
    
    return output;
}