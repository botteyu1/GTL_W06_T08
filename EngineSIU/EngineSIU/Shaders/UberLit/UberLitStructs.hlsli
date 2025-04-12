///////////////////////////////////////////////
// Shader Input Output
///////////////////////////////////////////////
struct VS_IN
{
    float3 position : POSITION; // 버텍스 위치
    float3 normal : NORMAL; // 버텍스 노멀
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
    float3 normal : NORMAL; // 월드 공간 노멀
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그 (1.0 또는 0.0)
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
    float3x3 TBN : TBN;
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
///////////////////////////////////////////////
struct FAmbientLight
{
    float3 AmbientColor;
    float Intensity;
    
    int bVisible;
    float3 Pad0;
};

struct FDirectionalLight
{
    float3 DirectionalColor;
    float Intensity;

    float3 Direction;
    float Pad0;

    int bVisible;
    float3 Pad1;
};

struct FPointLight
{
    float3 PointColor;
    float Intensity;

    float3 Position;
    float Attenuation;

    float Falloff;
    float3 Pad0;

    int bVisible;
    float3 Pad1;
};

struct FSpotLight
{
    float3 Color;
    float Intensity;

    float AttenuationRadius;
    float InnerConeAngle;
    float OuterConeAngle;
    float Falloff;

    float3 Direction;
    float Pad0;

    float3 Position;
    int bVisible;
};

///////////////////////////////////////////////
// Lighting 내부계산용
///////////////////////////////////////////////
struct SurfaceInfo
{
    float3 WorldPos;
    float3 Normal;
    float3 ViewDir; // normalize(CameraPos - WorldPos)
};

struct BlinnPhongParams
{
    float3 LightDir; // Directional일 경우: normalize(-Dir), Spot일 경우: normalize(Pos - P)
    float3 LightColor;
    float3 HalfVector;
    float Attenuation; // 거리, 각도 등 반영한 감쇠 계수
    float Shininess;
};
