///////////////////////////////////////////////
// Shader Input Output
///////////////////////////////////////////////
struct VS_IN
{
    float3 position : POSITION; // 버텍스 위치
    float3 Normal : NORMAL; // 버텍스 노멀
    float3 Tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR; // 버텍스 색상
    int materialIndex : MATERIAL_INDEX;
};

struct VS_OUT
{
    float4 position : SV_POSITION; // 클립 공간으로 변환된 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치 (조명용)
    float4 color : COLOR; // 버텍스 컬러 또는 머티리얼 베이스 컬러
    float3 Normal : NORMAL; // 월드 공간 노멀
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그 (1.0 또는 0.0)
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
    float3 Tangent : TEXCOORD3;
    float3 BiTangent : TEXCOORD4;
};

struct PS_OUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
};

///////////////////////////////////////////////
// Material
///////////////////////////////////////////////
struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    
    float3 AmbientColor;
    float DensityScalar;
    
    float3 SpecularColor;
    float SpecularScalar;
    
    float3 EmissiveColor;
    uint TextureFlag;

    float Sharpness;
    float Illum;

    float2 pad;
};

///////////////////////////////////////////////
// Lighting 상수버퍼용
// Define.h 참고해서 맞춰두기
///////////////////////////////////////////////
struct FAmbientLight
{
    float3 Color;
    float Intensity;
};


struct FDirectionalLight
{
    float3 Color;
    float Intensity;

    float3 Direction;
    float pad;
};

struct FPointLight
{
    float3 Color;
    float Intensity;

    float3 Position;
    float AttenuationRadius;

    float Falloff;
    float3 pad;
};

struct FSpotLight
{
    float3 Color;
    float Intensity;

    float3 Position;
    float AttenuationRadius;

    float3 Direction;
    float Falloff;
    
    float InnerConeAngle;
    float OuterConeAngle;
    float2 pad;
};

struct FTileLightIndex
{
    uint LightCount;
    uint LightIndices[MAX_NUM_INDICES_PER_TILE];
};

struct FScreenInfo
{
    row_major matrix ProjInv;
    row_major matrix ViewMatrix;
    row_major matrix ViewMatrixInv;
    uint NumTileWidth;
    uint NumTileHeight;
    uint TileSize;
    uint ScreenWidth;
    uint ScreenHeight;
    uint ScreenTopPadding;
    uint MaxNumPointLight;
    uint ScreenInfoPad;
};
