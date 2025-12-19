#include "Shared.hlsli"

Texture2D g_gameHDR : register(t0);

float3 LinearToSRGB(float3 linearColor)
{
    return pow(linearColor, 1.0f / 2.2f);
}

float4 main(PS_INPUT_QUAD input) : SV_Target
{
     // 1. 선형 HDR 값 로드 (Nits 값으로 간주)
    float3 C_linear709 = g_gameHDR.Sample(g_samLinear, input.tex).rgb;
    
    float exposureFactor = pow(2.0f, g_exposure);
    C_linear709 *= exposureFactor;

    float3 C_tonemapped;
    C_tonemapped = ACESFilm(C_linear709);
   
    float3 C_final;
    C_final = LinearToSRGB(C_tonemapped);
    return float4(C_final, 1.0);
}
