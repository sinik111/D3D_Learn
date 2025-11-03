SamplerState g_samLinear : register(s0);

cbuffer Transform : register(b0)
{
    matrix g_world;
    matrix g_view;
    matrix g_projection;
    uint g_refBoneIndex;
    float3 __pad1;
}

cbuffer Environment : register(b1)
{
    float4 g_cameraPos;
    float4 g_lightDir;
    float4 g_lightColor;
    float4 g_ambientLightColor;
}

cbuffer Material : register(b2)
{
    float4 g_materialAmbient;
    float4 g_materialSpecular;
    float g_shininess;
    float3 __pad2;
}

cbuffer BonePoseMatrix : register(b3)
{
    matrix g_bonePose[128];
}

cbuffer BoneOffsetMatrix : register(b4)
{
    matrix g_boneOffset[128];
}

struct VS_INPUT_SKINNING
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float3 binorm : BINORMAL;
    uint4 blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHT;
};

struct VS_INPUT_COMMON
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float3 binorm : BINORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float3 tan : TEXCOORD2;
    float3 binorm : TEXCOORD3;
    float3 worldPos : TEXCOORD4;
};

struct VS_INPUT_SKYBOX
{
    float3 pos : POSITION;
};

struct PS_INPUT_SKYBOX
{
    float4 pos : SV_Position;
    float3 localPos : TEXCOORD0;
};

float3 EncodeNormal(float3 n)
{
    return n * 0.5f + 0.5f;
}

float3 DecodeNormal(float3 n)
{
    return n * 2.0f - 1.0f;
}