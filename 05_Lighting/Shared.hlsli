
Texture2D texDiffuse : register(t0);
TextureCube texCubeDiffuse : register(t1);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix normalMatrix;
    
    float4 lightDir;
    float4 lightColor;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float3 norm : TEXCOORD0;
    float2 tex : TEXCOORD1;
};

struct PS_SKYBOX_INPUT
{
    float4 pos : SV_Position;
    float3 originPos : TEXCOORD0;
};