#ifndef SHADOW_MAPPING_HLSLI
#define SHADOW_MAPPING_HLSLI

#include "Shared.hlsli"

Texture2D g_texShadowMap : register(t5);

float CalculateShadowFactor(PS_INPUT_SHADOW input)
{
    float shadowFactor = 1.0f;
    if (g_useShadow)
    {
        float currentShadowDepth = input.lightViewPos.z / input.lightViewPos.w;
        float2 shadowMapUV = input.lightViewPos.xy / input.lightViewPos.w;
    
        shadowMapUV.y = -shadowMapUV.y;
        shadowMapUV = shadowMapUV * 0.5f + 0.5f;
    
        if (shadowMapUV.x >= 0.0f && shadowMapUV.x <= 1.0f && shadowMapUV.y >= 0.0f && shadowMapUV.y <= 1.0f)
        {
            #ifdef USE_SHADOW_PCF
            {
                float2 offsets[9] =
                {
                    float2(-1.0f, -1.0f), float2(0.0f, -1.0f), float2(1.0f, -1.0f),
                float2(-1.0f, 0.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f),
                float2(-1.0f, 1.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f)
                };
            
                float texelSize = 1.0f / g_shadowMapSize;
            
                shadowFactor = 0.0f;
                for (int i = 0; i < 9; ++i)
                {
                    float2 sampleUV = shadowMapUV + offsets[i] * texelSize;
                    shadowFactor += g_texShadowMap.SampleCmpLevelZero(g_samComparison, sampleUV, currentShadowDepth - 0.001f);
                }

                shadowFactor = shadowFactor / 9.0f;
            }
            #else
            {
                float sampleShadowDepth = g_texShadowMap.Sample(g_samLinear, shadowMapUV).r;
                if (currentShadowDepth > sampleShadowDepth + 0.001f)
                {
                    shadowFactor = 0.0f;
                }
            }
            #endif //USE_SHADOW_PCF
        }
    }
    
    return shadowFactor;
}

#endif //SHADOW_MAPPING_HLSLI