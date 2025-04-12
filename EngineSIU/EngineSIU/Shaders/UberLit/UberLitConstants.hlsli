// UberLitConstants.hlsl
#include "UberLit/UberLitStructs.hlsli"
// constant buffer를 지정하는 header file

// 전처리기 macro는 D3DCompile에서 정의

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float pad;
};
cbuffer cbLights : register(b2)
{
    FAmbientLight AmbientLight;

    FDirectionalLight DirectionalLights[MAX_NUM_DIR_LIGHTS];

    FPointLight PointLights[MAX_NUM_POINT_LIGHTS];

    FSpotLight SpotLights[MAX_NUM_SPOT_LIGHTS];

    int NumDirectionalLights;
    int NumPointLights;
    int NumSpotLights;
};
cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}
cbuffer FlagConstants : register(b4)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b5)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b6)
{
    float2 UVOffset;
    float2 TexturePad0;
}
