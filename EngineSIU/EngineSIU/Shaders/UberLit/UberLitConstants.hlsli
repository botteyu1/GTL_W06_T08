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

    FDirectionalLight DirectionalLights[NUM_MAX_DIRLIGHT]; // 현재 Light.hlsl는 1개만 쓰고있음.

    FPointLight PointLights[NUM_MAX_POINTLIGHT];

    FSpotLight SpotLights[NUM_MAX_SPOTLIGHT];

    int NumDirLights;
    int NumPointLights;
    int NumSpotLights;
    int Pad2;
};

// LightCulling 용 Pointlight
StructuredBuffer<FPointLight> PointLightBufferList : register(t16);
StructuredBuffer<FTileLightIndex> TileLightIndicesList : register(t17);

cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}
cbuffer FlagConstants : register(b4)
{
    // 0 : Unlit
    // 1 : lit
    // 2 : depth
    // 3 : normal
    // 4 : position
    // 15 까지 reserved
    int RenderFlag;
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

cbuffer ScreenInfo : register(b10)
{
    FScreenInfo ScreenInfo;
};
