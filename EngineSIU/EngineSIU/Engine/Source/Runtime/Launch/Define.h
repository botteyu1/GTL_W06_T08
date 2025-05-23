#pragma once
#include <cmath>
#include <algorithm>
#include "Core/Container/String.h"
#include "Core/Container/Array.h"
#include "UObject/NameTypes.h"

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


#define _TCHAR_DEFINED
#include <d3d11.h>

#include <Math/Color.h>

#include "Container/Map.h"

struct FStaticMeshVertex
{
    float X, Y, Z;    // Position
    float NormalX, NormalY, NormalZ;
    float TangentX, TangentY, TangentZ;
    float U = 0, V = 0;
    float R, G, B, A; // Color
    uint32 MaterialIndex;
};

// Material Subset
struct FMaterialSubset
{
    uint32 IndexStart; // Index Buffer Start pos
    uint32 IndexCount; // Index Count
    uint32 MaterialIndex; // Material Index
    FString MaterialName; // Material Name
};

struct FStaticMaterial
{
    class UMaterial* Material;
    FName MaterialSlotName;
    //FMeshUVChannelInfo UVChannelData;
};

// OBJ File Raw Data
struct FObjInfo
{
    FWString ObjectName; // OBJ File Name. Path + FileName.obj 
    FWString FilePath; // OBJ File Paths
    FString DisplayName; // Display Name
    FString MatName; // OBJ MTL File Name

    // Group
    uint32 NumOfGroup = 0; // token 'g' or 'o'
    TArray<FString> GroupName;

    // Vertex, UV, Normal List
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // Faces
    TArray<int32> Faces;

    // Index
    TArray<uint32> VertexIndices;
    TArray<uint32> NormalIndices;
    TArray<uint32> UVIndices;

    // Material
    TArray<FMaterialSubset> MaterialSubsets;
};

// TODO: 좀 더 좋은 위치 혹은 좋은 이름이 있으면 변경
namespace ETextureFlag
{
    enum Type : uint32 {
        None = 0,
        Diffuse = (1U << 0),
        Ambient = (1U << 1),
        Specular = (1U << 2),
        Alpha = (1U << 3),
        Emissive = (1U << 4),
        Roughness = (1U << 5),
        Normal = (1U << 6),
        // Displacement = (1U << 7),
        // StencilDecal = (1U << 8),
    };
}

struct PerTextureData
{
    FString Name;
    FWString Path;
};

struct FObjMaterialInfo
{
    FString MaterialName;  // newmtl : Material Name.

    bool bTransparent = false; // Has alpha channel?

    uint32 TextureFlag = ETextureFlag::Type::None;
    
    FVector Diffuse;  // Kd : Diffuse (Vector4)
    FVector Specular;  // Ks : Specular (Vector) 
    FVector Ambient;   // Ka : Ambient (Vector)
    FVector Emissive;  // Ke : Emissive (Vector)
    
    float SpecularScalar; // Ns : Specular Power (Float)             Specular Exponent,  Shininess
    float Sharpness;  // 선명도
    // float roughness = clamp(1.0f - (Ns / 1000.0f), 0.0f, 1.0f);   or   float roughness = sqrt(2.0f / (Ns + 2.0f));
    float DensityScalar;  // Ni : Optical Density (Float)
    float TransparencyScalar; // d or Tr  : Transparency of surface (Float)

    uint32 IlluminanceModel; // illum: illumination Model between 0 and 10. (UINT)

    TMap<ETextureFlag::Type, PerTextureData> TextureData;
};

// Cooked Data
namespace OBJ
{
    struct FStaticMeshRenderData
    {
        FWString ObjectName;
        FString DisplayName;

        TArray<FStaticMeshVertex> Vertices;
        TArray<UINT> Indices;

        ID3D11Buffer* VertexBuffer;
        ID3D11Buffer* IndexBuffer;

        TArray<FObjMaterialInfo> Materials;
        TArray<FMaterialSubset> MaterialSubsets;

        FVector BoundingBoxMin;
        FVector BoundingBoxMax;
    };
}

struct FVertexTexture
{
    float x, y, z;    // Position
    float u, v; // Texture
};
struct FGridParameters
{
    float GridSpacing;
    int   NumGridLines;
    FVector GridOrigin;
    float pad;
};
struct FSimpleVertex
{
    float dummy; // 내용은 사용되지 않음
    float padding[11];
};
struct FOBB {
    FVector corners[8];
};

struct FPoint
{
    FPoint() : X(0), Y(0) {}
    FPoint(float _x, float _y) : X(_x), Y(_y) {}
    FPoint(long _x, long _y) : X(_x), Y(_y) {}
    FPoint(int _x, int _y) : X(_x), Y(_y) {}

    float X, Y;
};
struct FBoundingBox
{
    FBoundingBox() = default;
    FBoundingBox(FVector _min, FVector _max) : min(_min), max(_max) {}
    FVector min; // Minimum extents
    float pad;
    FVector max; // Maximum extents
    float pad1;

    inline FVector GetPosition() { return (min + max) / 2.f; }
    inline FVector GetExtent() { return (max - min) / 2.f; }

    bool Intersect(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const
    {
        float tmin = -FLT_MAX;
        float tmax = FLT_MAX;
        constexpr float epsilon = 1e-6f;

        // X축 처리
        if (fabs(rayDir.X) < epsilon)
        {
            // 레이가 X축 방향으로 거의 평행한 경우,
            // 원점의 x가 박스 [min.X, max.X] 범위 밖이면 교차 없음
            if (rayOrigin.X < min.X || rayOrigin.X > max.X)
                return false;
        }
        else
        {
            float t1 = (min.X - rayOrigin.X) / rayDir.X;
            float t2 = (max.X - rayOrigin.X) / rayDir.X;
            if (t1 > t2)  std::swap(t1, t2);

            // tmin은 "현재까지의 교차 구간 중 가장 큰 min"
            tmin = (t1 > tmin) ? t1 : tmin;
            // tmax는 "현재까지의 교차 구간 중 가장 작은 max"
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // Y축 처리
        if (fabs(rayDir.Y) < epsilon)
        {
            if (rayOrigin.Y < min.Y || rayOrigin.Y > max.Y)
                return false;
        }
        else
        {
            float t1 = (min.Y - rayOrigin.Y) / rayDir.Y;
            float t2 = (max.Y - rayOrigin.Y) / rayDir.Y;
            if (t1 > t2)  std::swap(t1, t2);

            tmin = (t1 > tmin) ? t1 : tmin;
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // Z축 처리
        if (fabs(rayDir.Z) < epsilon)
        {
            if (rayOrigin.Z < min.Z || rayOrigin.Z > max.Z)
                return false;
        }
        else
        {
            float t1 = (min.Z - rayOrigin.Z) / rayDir.Z;
            float t2 = (max.Z - rayOrigin.Z) / rayDir.Z;
            if (t1 > t2)  std::swap(t1, t2);

            tmin = (t1 > tmin) ? t1 : tmin;
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // 여기까지 왔으면 교차 구간 [tmin, tmax]가 유효하다.
        // tmax < 0 이면, 레이가 박스 뒤쪽에서 교차하므로 화면상 보기엔 교차 안 한다고 볼 수 있음
        if (tmax < 0.0f)
            return false;

        // outDistance = tmin이 0보다 크면 그게 레이가 처음으로 박스를 만나는 지점
        // 만약 tmin < 0 이면, 레이의 시작점이 박스 내부에 있다는 의미이므로, 거리를 0으로 처리해도 됨.
        outDistance = (tmin >= 0.0f) ? tmin : 0.0f;

        return true;
    }

};
struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    FVector4 Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];

};
struct FPrimitiveCounts
{
    int BoundingBoxCount;
    int pad;
    int ConeCount;
    int pad1;
};

constexpr uint32 NUM_MAX_DIRLIGHT = 1;
constexpr uint32 NUM_MAX_POINTLIGHT = 16;
constexpr uint32 NUM_MAX_SPOTLIGHT = 8;

enum ELightType {
    POINT_LIGHT = 1,
    SPOT_LIGHT = 2,
    DIRECTIONAL_LIGHT = 3,
    AMBIENT_LIGHT = 4,
};

struct alignas(16) FAmbientLight
{
    FVector Color;
    float Intensity;
};

struct alignas(16) FDirectionalLight
{
    FVector Color;
    float Intensity;

    FVector Direction;
    // pad 4 byte
};

struct alignas(16) FPointLight
{
    FVector Color;
    float Intensity;

    FVector Position;
    float AttenuationRadius;

    float Falloff;
    FVector pad;
    // pad 12 byte
};

struct alignas(16) FSpotLight
{
    FVector Color;
    float Intensity;

    FVector Position;  
    float AttenuationRadius;

    FVector Direction;
    float Falloff;

    float InnerConeAngle;
    float OuterConeAngle;
    // pad 8 byte
};

struct alignas(16) FSceneLightBuffer
{
    FAmbientLight AmbientLight;

    FDirectionalLight DirectionalLight[NUM_MAX_DIRLIGHT];
    FPointLight PointLight[NUM_MAX_POINTLIGHT];
    FSpotLight SpotLight[NUM_MAX_SPOTLIGHT];
    
    int NumDirLights;
    int NumPointLights;
    int NumSpotLights;
    int pad;
    // pad 4 byte
};

struct alignas(16) FMaterialConstants {
    FVector DiffuseColor;
    float TransparencyScalar;

    FVector AmbientColor;
    float DensityScalar;

    FVector SpecularColor;
    float SpecularScalar;

    FVector EmmisiveColor;
    uint32 TextureFlag;

    float Sharpness;
    float Illum;

    float pad1;
    float pad2;
};

struct alignas(16) FPerObjectConstantBuffer {
    FMatrix Model;      // 모델
    FMatrix ModelMatrixInverseTranspose; // normal 변환을 위한 행렬
    FVector4 UUIDColor;
    int IsSelected;
    FVector pad;
};

struct alignas(16) FCameraConstantBuffer
{
    FMatrix View;
    FMatrix Projection;
    FVector CameraPosition;
    float pad;
};

struct alignas(16)FSubUVConstant
{
    FVector2D uvOffset;
    FVector2D uvScale;
};
struct alignas(16)FFlagConstants {
    // 0 : Unlit
    // 1 : lit
    // 2 : depth
    // 3 : normal
    // 4 : position
    // 15 까지 reserved
    int RenderFlag;
    FVector pad;
};

struct alignas(16) FSubMeshConstants {
    float isSelectedSubMesh;
    FVector pad;
};

struct alignas(16) FTextureConstants {
    float UOffset;
    float VOffset;
    float pad0;
    float pad1;
};

struct FLinePrimitiveBatchArgs
{
    FGridParameters GridParam;
    ID3D11Buffer* VertexBuffer;
    int BoundingBoxCount;
    int ConeCount;
    int ConeSegmentCount;
    int OBBCount;
};

struct FVertexInfo
{
    uint32_t NumVertices;
    uint32_t Stride;
    ID3D11Buffer* VertexBuffer;
};

struct FIndexInfo
{
    uint32_t NumIndices;
    ID3D11Buffer* IndexBuffer;
};

struct FBufferInfo
{
    FVertexInfo VertexInfo;
    FIndexInfo IndexInfo;
};

struct FScreenConstants
{
    FVector2D ScreenSize;   // 화면 전체 크기 (w, h)
    FVector2D UVOffset;     // 뷰포트 시작 UV (x/sw, y/sh)
    FVector2D UVScale;      // 뷰포트 크기 비율 (w/sw, h/sh)
    FVector2D Padding;      // 정렬용 (사용 안 해도 무방)
};


struct alignas(16) FFogConstants
{
    FMatrix InvViewProj;
    FLinearColor FogColor;
    FVector CameraPos;
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    FVector FogPosition;
    float CameraNear;
    float CameraFar;
    FVector padding;
};


// Spotlight Light Culling을 위한 값
// CPU -> GPU 전달용 scene의 전체 light를 담은 버퍼
constexpr static uint32 MaxNumPointLight = 32768; // define에서 바꾸기
constexpr char MaxNumPointLightChar[6] = "32768";

// 64개 - 개수담을 1개
constexpr static uint32 MaxNumPointLightPerTile = 127;
constexpr char MaxNumPointLightPerTileChar[4] = "127";
