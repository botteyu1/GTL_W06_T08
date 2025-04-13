// UberLitConstants.hlsl
#include "UberLitStructs.hlsli"
// constant buffer를 지정하는 header file

// 전처리기 macro는 D3DCompile에서 정의

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 Pad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float Pad1;
};
cbuffer cbLights : register(b2)
{
    FAmbientLight AmbientLight;

    FDirectionalLight DirectionalLights[NUM_MAX_DIRLIGHT]; // 현재 Light.hlsl는 1개만 쓰고있음.

    FPointLight PointLights[NUM_MAX_POINTLIGHT];

    FSpotLight SpotLights[NUM_MAX_SPOTLIGHT];

    int NumDirLights;
    int NumPointLights;
    int NumSpotLights;
    int Pad2;
};
cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}
cbuffer FlagConstants : register(b4)
{
    bool IsLit;
    float3 Pad3;
}

cbuffer SubMeshConstants : register(b5)
{
    bool IsSelectedSubMesh;
    float3 Pad4;
}

cbuffer TextureConstants : register(b6)
{
    float2 UVOffset;
    float2 Pad5;
}
