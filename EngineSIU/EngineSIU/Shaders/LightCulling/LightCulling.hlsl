// StructuredBuffer로 Light 받음
#include "CalculateFunctions.hlsli"

struct FPointLight
{
    float3 Color;
    float Intensity;

    float3 Position;
    float AttenuationRadius;

    float Falloff;
    float3 pad;
};

struct FTileLightIndex
{
    uint LightCount;
    float LightIndices[31];
};

cbuffer ScreenInfo : register(b10)
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

StructuredBuffer<FPointLight> PointLightBufferList : register(t0);
RWStructuredBuffer<FTileLightIndex> TileLightIndicesListCS : register(u1);

[numthreads(1, 1, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID)
{
    uint2 tileCoord = DTid.xy;
    uint tileIndex = tileCoord.y * NumTileWidth + tileCoord.x;

    uint2 TileUpLeft = tileCoord * TileSize;
    uint Right = clamp(TileUpLeft.x + TileSize, 0, ScreenWidth);
    uint Bottom = clamp(TileUpLeft.y + TileSize, 0, ScreenHeight);
    
    uint2 TileUpRight = TileUpLeft;
    TileUpRight.x = Right;
    uint2 TileBottomLeft = TileUpLeft;
    TileBottomLeft.y = Bottom;
    uint2 TileBottomRight = TileUpLeft;
    TileBottomRight.x = Right;
    TileBottomRight.y = Bottom;
    
    float2 screenSize = float2(ScreenWidth, ScreenHeight);

// D3D 기준에 맞는 방식
    float2 ndcUL = float2(
        (float) TileUpLeft.x / screenSize.x * 2.0f - 1.0f,
        1.0f - (float) TileUpLeft.y / screenSize.y * 2.0f
    );
    float2 ndcUR = float2(
        (float) TileUpRight.x / screenSize.x * 2.0f - 1.0f,
        1.0f - (float) TileUpRight.y / screenSize.y * 2.0f
    );
    float2 ndcLL = float2(
        (float) TileBottomLeft.x / screenSize.x * 2.0f - 1.0f,
        1.0f - (float) TileBottomLeft.y / screenSize.y * 2.0f
    );
    float2 ndcLR = float2(
        (float) TileBottomRight.x / screenSize.x * 2.0f - 1.0f,
        1.0f - (float) TileBottomRight.y / screenSize.y * 2.0f
    );
    

    float2 NDC[4] = { ndcUL, ndcUR, ndcLL, ndcLR };
    // frustum을 구성하는 여섯개의 평면을 생성
    float4 Planes[6];
    GetTileFrustumPlanes(NDC, ProjInv, ViewMatrixInv, Planes);
    
    float3 RayOrigin[4];
    float3 RayDir[4];
    // 코너에 4개의 ray 생성
    for (int j = 0; j < 4; ++j)
    {
        matrix viewprojinv = mul(ProjInv, ViewMatrixInv);
        float4 rayOrigin = float4(NDC[j], 0, 1);
        float4 rayFar = float4(NDC[j], 1, 1);
        rayOrigin = mul(rayOrigin, viewprojinv);
        rayFar = mul(rayFar, viewprojinv);
    
        rayOrigin /= rayOrigin.w;
        rayFar /= rayFar.w;
            
        RayDir[j] = (rayFar - rayOrigin).xyz;
        RayOrigin[j] = rayOrigin.xyz;
    }
    

    int NumIntersection = 0;
    for (uint i = 0; i < MaxNumPointLight; ++i)
    {
        if (PointLightBufferList[i].AttenuationRadius <= 0)
        {
            continue;
        }
        // frustum 방식은 false positive 발생
        // true negative만 reject
        if (!SphereFrustumPlaneIntersection(PointLightBufferList[i].Position, PointLightBufferList[i].AttenuationRadius, Planes))
        {
            continue;
        }
        
        for (int j = 0; j < 4; ++j)
        {
            if (IntersectRaySphere(RayOrigin[j], RayDir[j], PointLightBufferList[i].Position, PointLightBufferList[i].AttenuationRadius))
            {
                NumIntersection++;
                break;
            }
        }

    }
    
    //TileLightIndicesListCS[tileIndex].LightIndices[0] = -1000000*Planes[1].x;

    
    TileLightIndicesListCS[tileIndex].
        LightCount = NumIntersection;
    TileLightIndicesListCS[tileIndex].LightIndices[1] = SphereFrustumPlaneIntersection(PointLightBufferList[0].Position, PointLightBufferList[0].AttenuationRadius, Planes);
    

}

struct PSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

PSInput mainVS(uint vertexID : SV_VertexID)
{
    PSInput output;

    // Clip-space 정점
    float2 positions[4] =
    {
        float2(-1.0f, 1.0f),
        float2(1.0f, 1.0f),
        float2(-1.0f, -1.0f),
        float2(1.0f, -1.0f)
    };

    float2 uvs[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f)
    };

    output.pos = float4(positions[vertexID], 0.0f, 1.0f);
    output.uv = uvs[vertexID];

    return output;
}

StructuredBuffer<FTileLightIndex> TileLightIndicesListPS : register(t1); // t1로 버퍼를 연결

float4 mainPS(PSInput input) : SV_Target
{
    // UI를 제외한 위치의 픽셀 위치
    // window가 아니라 작은 사각형의 topleft 기준
    uint2 pixelCoord = uint2(input.pos.xy) - uint2(0, ScreenTopPadding);

    // tile 위치 계산
    uint2 tileCoord = pixelCoord / TileSize;
    uint tileIndex = tileCoord.y * NumTileWidth + tileCoord.x;

    uint lightCount = TileLightIndicesListPS[tileIndex].LightCount;

    // light count를 색상으로 매핑 (예: 최대 32개 기준)
    float intensity = saturate(lightCount / 31.0f);
    
    //if (TileLightIndicesListPS[tileIndex].LightIndices[1] == 0)
    //{
    //    return float4(1, 1, 1, 1);

    //}
    return float4(
    HeatmapColor(TileLightIndicesListPS[tileIndex].LightCount, 0, 40),
    0.5f
    );
    return float4(
    TileLightIndicesListPS[tileIndex].LightCount / 8000.f,
    TileLightIndicesListPS[tileIndex].LightIndices[1] / 8.f,
    TileLightIndicesListPS[tileIndex].LightIndices[2] / 8.f,
    0.3f);
    // 빨간색 톤으로 intensity 표시
    //return float4(1, 1, 0, 1);
    //return float4(tileCoord / float2 (NumTileWidth-1,NumTileHeight - 1), 0.0f, 1.0f);
    //if (intensity > 0)
    //{
    //    return float4(intensity, 0, 0, 0.5);
    //}
    //else
    //{
    //    return float4(intensity, 0, 0, 0.5);
    //    return float4(1, 0, 0, 1);
    //}
}
