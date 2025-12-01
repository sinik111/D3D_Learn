#include "Shared.hlsli"

PS_INPUT_SKYBOX main(VS_INPUT_SKYBOX input)
{
    PS_INPUT_SKYBOX output = (PS_INPUT_SKYBOX) 0;
    
    float4x4 viewRotation = g_view;
    viewRotation._41_42_43 = 0.0f;
    
    output.pos = mul(float4(input.pos, 0.0f), g_world);
    output.pos = mul(output.pos, viewRotation);
    output.pos = mul(output.pos, g_projection).xyww;
    
    output.localPos = input.pos;
    
    return output;
}