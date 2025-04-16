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
    uint LightIndices[MAX_NUM_INDICES_PER_TILE];
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

// 32개의 thread가 lockstep으로 계산하므로, 32개로 나눔
groupshared uint SharedLightIndices[MAX_NUM_INDICES_PER_TILE];
groupshared uint SharedLightCount;

#define NumThread 64

[numthreads(NumThread, 1, 1)]
void mainCS(uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID, uint3 GroupID : SV_GroupID)
{
    uint localThreadID = GTid.x;

    uint2 tileCoord = GroupID.xy;
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
    
    float3 RayOrigin[5];
    float3 RayDir[5];
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
    // 중점에 한개의 ray 생성
    RayDir[4] = (RayDir[0] + RayDir[3]) / 2.f;
    RayOrigin[4] = (RayOrigin[0] + RayOrigin[3]) / 2.f;

    // 가장 앞의 쓰레드가 값을 컨트롤함
    if (localThreadID == 0)
    {
        SharedLightCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    int NumIntersection = 0;
    for (uint LightIndex = localThreadID; LightIndex < MaxNumPointLight; LightIndex += NumThread)
    {
        // 쓰레기값은 버림 (heap에서 넘어와서 다 음수값)
        if (PointLightBufferList[LightIndex].AttenuationRadius <= 0)
        {
            continue;
        }
        // frustum 방식은 false positive 발생
        // true negative만 reject
        if (!SphereFrustumPlaneIntersection(PointLightBufferList[LightIndex].Position, PointLightBufferList[LightIndex].AttenuationRadius, Planes))
        {
            continue;
        }
        
        // if문에 넣지말고 밖에서 값 넣기
        bool intersects = false;
        // corner4개에서 나오는 ray와 intersection
        for (int j = 0; j < 5; ++j)
        {
            if (IntersectRaySphere(RayOrigin[j], RayDir[j], PointLightBufferList[LightIndex].Position, PointLightBufferList[LightIndex].AttenuationRadius))
            {
                intersects = true;
                break;
            }
        }
        
        if (intersects)
        {
            uint index;
            InterlockedAdd(SharedLightCount, 1, index);
            if (index < MAX_NUM_INDICES_PER_TILE)
            {
                SharedLightIndices[index] = LightIndex;
            }
        }
    }
    
    // thread0가 기록
    GroupMemoryBarrierWithGroupSync();
    if (localThreadID == 0)
    {
        TileLightIndicesListCS[tileIndex].LightCount = min(SharedLightCount, MAX_NUM_INDICES_PER_TILE);
        for (uint i = 0; i < min(SharedLightCount, MAX_NUM_INDICES_PER_TILE); ++i)
        {
            TileLightIndicesListCS[tileIndex].LightIndices[i] = SharedLightIndices[i];
        }
    }
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

StructuredBuffer<FTileLightIndex> TileLightIndicesListPS : register(t17); // t1로 버퍼를 연결

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
    
    return float4(
    HeatmapColor(TileLightIndicesListPS[tileIndex].LightCount, 0, MAX_NUM_INDICES_PER_TILE),
    0.5f
    );
}
