#include "Shared.hlsli"

float4 main(PS_INPUT_OUTLINE input) : SV_Target
{
	return input.color;
}