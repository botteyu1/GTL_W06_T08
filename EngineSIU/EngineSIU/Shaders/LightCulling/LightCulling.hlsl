// StructuredBuffer로 Light 받음
#include "CalculateFunctions.hlsli"

struct FPointLightBuffer
{
    float3 Position;
    float Radius;
};
struct FTileLightIndex
{
    uint LightCount;
    uint LightIndices[31];
};

cbuffer ScreenInfo : register(b10)
{
    row_major matrix ProjInv;
    row_major matrix ViewMatrix;
    uint NumTileWidth;
    uint NumTileHeight;
    uint TileSize;
    uint ScreenWidth;
    uint ScreenHeight;
    uint3 ScreenInfoPad;
};

StructuredBuffer<FPointLightBuffer> PointLightBufferList : register(t0);
RWStructuredBuffer<FTileLightIndex> TileLightIndicesListCS : register(u1);

[numthreads(1, 1, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID)
{
    uint2 tileCoord = DTid.xy;
    uint tileIndex = tileCoord.y * NumTileWidth + tileCoord.x;

    float2 TileUpLeft = tileCoord * TileSize;
    float Right = clamp(TileUpLeft.x + TileSize, 0, ScreenWidth - 1);
    float Bottom = clamp(TileUpLeft.y + TileSize, 0, ScreenHeight - 1);
    float2 TileUpRight = TileUpLeft;
    TileUpRight.x = Right;
    float2 TileBottomLeft = TileUpLeft;
    TileBottomLeft.y = Bottom;
    float2 TileBottomRight = TileUpLeft;
    TileBottomRight.x = Right;
    TileBottomRight.y = Bottom;
    
    float2 NDC[4]; // UL, UR, LL, LR
    NDC[0] = float2(TileUpLeft.x / ScreenWidth, -TileUpLeft.y / ScreenHeight) * 2 + float2(-1,1);
    NDC[1] = float2(TileUpRight.x / ScreenWidth, -TileUpRight.y / ScreenHeight) * 2 + float2(-1, 1);
    NDC[2] = float2(TileBottomLeft.x / ScreenWidth, -TileBottomLeft.y / ScreenHeight) * 2 + float2(-1, 1);
    NDC[3] = float2(TileBottomRight.x / ScreenWidth, -TileBottomRight.y / ScreenHeight) * 2 + float2(-1, 1);
    
    float3 ViewPos[4];
    for (int i = 0; i < 4; ++i)
    {
        float4 clip = float4(NDC[i], 1, 1); // z=1 (far plane)
        float4 view = mul(ProjInv, clip);
        ViewPos[i] = view.xyz / view.w;
    }
    
    // 3. 6개 평면 구하기
    float4 FrustumPlanes[6];
    ComputeFrustumPlanes(ViewPos, FrustumPlanes); // 이건 직접 구현해야 함
    
    FTileLightIndex tileLightData;
    tileLightData.LightCount = 0;
    for (int i = 0; i < 31; ++i)
    {
        tileLightData.LightIndices[i] = 0;
    }

    uint maxLightCount;
    uint lightCount = 0;
    uint stride;
    PointLightBufferList.GetDimensions(maxLightCount, stride);
    
    uint TempIndices[31];
    uint TempCount = 0;
    for (uint i = 0; i < MAX_NUM_GLOBAL_LIGHT; ++i)
    {
        FPointLightBuffer light = PointLightBufferList[i];
        // TArray.Reserve에서 해주면 -431602080.0000이 들어감
        if (light.Radius <= 0)
        {
            continue;
        }
        // Light를 View space로 변환
        float4 viewPos = mul(ViewMatrix, float4(light.Position, 1.0f));
        float3 viewSpaceLightPos = viewPos.xyz;

        // Frustum과의 교차 판정
        if (SphereInFrustum(viewSpaceLightPos, light.Radius, FrustumPlanes))
        {
            if (TempCount < 31)
            {
                TempIndices[TempCount] = i;
                TempCount++;
                //tileLightData.LightIndices[lightCount] = i;
                //lightCount++;
            }
        }
    }
    tileLightData.LightCount = 123;
    TileLightIndicesListCS[DTid.x] = tileLightData;
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
    // 픽셀 위치
    uint2 pixelCoord = uint2(input.pos.xy);

    // tile 위치 계산
    uint2 tileCoord = pixelCoord / TileSize;
    uint tileIndex = tileCoord.y * NumTileWidth + tileCoord.x;

    uint lightCount = TileLightIndicesListPS[tileIndex].LightCount;

    // light count를 색상으로 매핑 (예: 최대 32개 기준)
    float intensity = saturate(lightCount / 32.0f);

    // 빨간색 톤으로 intensity 표시
    return float4(1, tileIndex / (float) 5000, 0.0f, 1.0f);
}
