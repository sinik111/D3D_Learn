#include "Shared.hlsli"

PS_INPUT_SHADOW main(VS_INPUT_SKINNING input)
{
    PS_INPUT_SHADOW output = (PS_INPUT_SHADOW) 0;
    
    float4x4 offsetPose[4];
    offsetPose[0] = mul(g_boneOffset[input.blendIndices.x], g_bonePose[input.blendIndices.x]);
    offsetPose[1] = mul(g_boneOffset[input.blendIndices.y], g_bonePose[input.blendIndices.y]);
    offsetPose[2] = mul(g_boneOffset[input.blendIndices.z], g_bonePose[input.blendIndices.z]);
    offsetPose[3] = mul(g_boneOffset[input.blendIndices.w], g_bonePose[input.blendIndices.w]);
    
    float4x4 weightedOffsetPose;
    weightedOffsetPose = mul(input.blendWeights.x, offsetPose[0]);
    weightedOffsetPose += mul(input.blendWeights.y, offsetPose[1]);
    weightedOffsetPose += mul(input.blendWeights.z, offsetPose[2]);
    weightedOffsetPose += mul(input.blendWeights.w, offsetPose[3]);
    
    float4x4 world = mul(weightedOffsetPose, g_world);
    
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = output.pos.xyz;
    output.pos = mul(output.pos, g_view);
    output.pos = mul(output.pos, g_projection);
    
    output.norm = mul(input.norm, (float3x3) world);
    output.tan = mul(input.tan, (float3x3) world);
    output.binorm = mul(input.binorm, (float3x3) world);
    
    output.tex = input.tex;
    
    output.lightViewPos = mul(float4(output.worldPos, 1.0f), g_lightView);
    output.lightViewPos = mul(output.lightViewPos, g_lightProjection);
    
    return output;
}