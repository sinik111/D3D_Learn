#include "Shared.hlsli"

PS_INPUT_SKYBOX main(VS_INPUT_SKYBOX input)
{
    PS_INPUT_SKYBOX output = (PS_INPUT_SKYBOX) 0;
	
    output.pos = mul(float4(input.pos, 0.0f), g_projection);

    output.localPos = input.pos;
	
    return output;
}