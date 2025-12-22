#include "Shared.hlsli"

struct GBufferOut
{
    float4 baseColor : SV_Target0;
    float4 position : SV_Target1;
    float4 normal : SV_Target2;
    float4 emissive : SV_Target3;
    float4 orm : SV_Target4;
};

Texture2D g_texDiffuse : register(t0);
Texture2D g_texNormal : register(t1);
Texture2D g_texEmissive : register(t3);
Texture2D g_texOpacity : register(t4);
Texture2D g_texMetalness : register(t5);
Texture2D g_texRoughness : register(t6);
Texture2D g_texAmbientOcclusion : register(t7);

GBufferOut main(PS_INPUT input)
{
    GBufferOut output = (GBufferOut) 0;
    
    float opacityFactor = g_texOpacity.Sample(g_samLinear, input.tex).a;
    clip(opacityFactor - 0.5f);
    
    output.position = float4(input.worldPos, 1.0f);
    output.baseColor = float4(pow(g_texDiffuse.Sample(g_samLinear, input.tex).rgb, 2.2f), 1.0f);
    
    float3 texNorm = g_texNormal.Sample(g_samLinear, input.tex).rgb;
    output.emissive = float4(pow(g_texEmissive.Sample(g_samLinear, input.tex).rgb, 2.2f), 1.0f);
    output.orm.r = g_texAmbientOcclusion.Sample(g_samLinear, input.tex).r;
    output.orm.g = g_texRoughness.Sample(g_samLinear, input.tex).r;
    output.orm.b = g_texMetalness.Sample(g_samLinear, input.tex).r;
    output.orm.a = 1.0f;
        
    if (g_overrideMaterial)
    {
        output.baseColor = g_overrideBaseColor;
        output.orm.b = g_overrideMetalness;
        output.orm.g = g_overrideRoughness;
    }
    
    output.orm.r = max(EPSILON, output.orm.r);
    
    // normal
    float3x3 tbn = float3x3(normalize(input.tan), normalize(input.binorm), normalize(input.norm));
    float3 n = normalize(mul(DecodeNormal(texNorm), tbn));
    output.normal = float4(EncodeNormal(n), 1.0f);
    
    return output;
}