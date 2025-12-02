#include "Shared.hlsli"

Texture2D g_texDiffuse : register(t0);
Texture2D g_texNormal : register(t1);
Texture2D g_texSpecular : register(t2);
Texture2D g_texEmissive : register(t3);
Texture2D g_texOpacity : register(t4);
Texture2D g_texMetalness : register(t5);
Texture2D g_texRoughness : register(t6);
Texture2D g_texShadowMap : register(t7);

static const float3 DielectricFactor = 0.04f;

float NDFGGXTR(float nDotH, float roughness)
{
    float a = roughness * roughness;
    float aSq = a * a;
    
    float denom = (nDotH * nDotH) * (aSq - 1.0f) + 1.0f;
    
    return aSq / (PI * denom * denom);
}

float3 FresnelSchlick(float3 f0, float cosTheta)
{
    return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

float GAFSchlickGGXSub(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0f - k) + k);
}

float GAFSchlickGGX(float nDotV, float nDotL, float roughness)
{
    float a = roughness + 1.0f;
    float k = (a * a) / 8.0f;
    
    return GAFSchlickGGXSub(nDotV, k) * GAFSchlickGGXSub(nDotL, k);
}

float4 main(PS_INPUT_SHADOW input) : SV_Target
{
    float3 texDiffColor = pow(g_texDiffuse.Sample(g_samLinear, input.tex).rgb, 2.2f);
    //float3 texDiffColor = g_texDiffuse.Sample(g_samLinear, input.tex).rgb;
    float4 texNormColor = g_texNormal.Sample(g_samLinear, input.tex);
    float4 texSpecColor = g_texSpecular.Sample(g_samLinear, input.tex);
    float opacityFactor = g_texOpacity.Sample(g_samLinear, input.tex).a;
    float4 texEmsvColor = g_texEmissive.Sample(g_samLinear, input.tex);
    float metalnessFactor = g_texMetalness.Sample(g_samLinear, input.tex).r;
    float roughnessFactor = g_texRoughness.Sample(g_samLinear, input.tex).r;
    
    if (g_overrideMaterial)
    {
        texDiffColor = g_overrideBaseColor;
        metalnessFactor = g_overrideMetalness;
        roughnessFactor = g_overrideRoughness;
    }
    
    clip(opacityFactor - 0.5f);
    
    // emissive
    float4 emissive = texEmsvColor;
    
    // normal
    float3x3 tbn = float3x3(normalize(input.tan), normalize(input.binorm), normalize(input.norm));
    float3 n = normalize(mul(DecodeNormal(texNormColor.rgb), tbn));
    
    // view
    float3 v = normalize(g_cameraPos - input.worldPos);
    
    // light
    float3 l = -g_lightDir;
    
    // l-v half
    float3 h = normalize(l + v);
    
    // n dot h
    float nDotH = max(0.0f, dot(n, h));
    
    // n dot l
    float nDotL = max(0.0f, dot(n, l));
    
    // n dot v
    float nDotV = max(0.0f, dot(n, v));
    
    // h dot v
    float hDotV = max(0.0f, dot(h, v));
    
    // shadow
    float shadowFactor = 1.0f;
    float currentShadowDepth = input.lightViewPos.z / input.lightViewPos.w;
    float2 shadowMapUV = input.lightViewPos.xy / input.lightViewPos.w;
    
    shadowMapUV.y = -shadowMapUV.y;
    shadowMapUV = shadowMapUV * 0.5f + 0.5f;
    
    if (shadowMapUV.x >= 0.0f && shadowMapUV.x <= 1.0f && shadowMapUV.y >= 0.0f && shadowMapUV.y <= 1.0f)
    {
        if (g_useShadowPCF)
        {
            if (currentShadowDepth > 1.0f)
            {
                shadowFactor = 1.0f;
            }
            else
            {
                float texelSize = 1.0f / g_shadowMapSize;
                shadowFactor = 0.0f;

                int max = 1;
                
                for (int y = -max; y <= max; ++y)
                {
                    for (int x = -max; x <= max; ++x)
                    {
                        float2 offset = float2(x, y) * texelSize;
                        float2 sampleUV = shadowMapUV + offset;
                        shadowFactor += g_texShadowMap.SampleCmpLevelZero(g_samComparison, sampleUV, currentShadowDepth - 0.0001f);
                    }
                }
                shadowFactor = shadowFactor / ((max * 2 + 1) * (max * 2 + 1));
            }
        }
        else
        {
            float sampleShadowDepth = g_texShadowMap.Sample(g_samLinear, shadowMapUV).r;
            if (currentShadowDepth > 1.0f)
            {
                shadowFactor = 1.0f;
            }
            else if (currentShadowDepth > sampleShadowDepth + 0.0001f)
            {
                shadowFactor = 0.0f;
            }
        }
    }
    
    float d = NDFGGXTR(nDotH, roughnessFactor);
    
    float3 f0 = lerp(DielectricFactor, texDiffColor, metalnessFactor);
    float3 f = FresnelSchlick(f0, hDotV);
    
    float g = GAFSchlickGGX(nDotV, nDotL, roughnessFactor);
    
    float3 kd = lerp(1.0f - f, 0.0f, metalnessFactor);
    
    float3 diffuseBRDF = kd * texDiffColor / PI;
    float3 specularBRDF = (f * d * g) / max(EPSILON, 4.0f * nDotL * nDotV);
    
    float3 directLighting = (diffuseBRDF + specularBRDF) * (float3) g_lightColor * nDotL * shadowFactor;
    
    float3 indirectDiffuse = kd * texDiffColor * (float3) g_ambientLightColor;
    
    float3 totalLighting = directLighting + indirectDiffuse;
    
    return float4(pow(directLighting, 1.0f / 2.2f), 1.0f);
}