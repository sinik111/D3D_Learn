#include "Shared.hlsli"

PS_SKYBOX_INPUT main(VS_INPUT input)
{
    PS_SKYBOX_INPUT output = (PS_SKYBOX_INPUT) 0;
	
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection).xyww;

    output.originPos = input.pos;
	
	return output;
}