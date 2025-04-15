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
    uint2 ScreenInfoPad;
};

StructuredBuffer<FPointLightBuffer> PointLightBufferList : register(t0);
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
    

    // uint 마이너스 붙이면 쓰레기값
    //// Step 1: NDC 좌표
    //float2 ndcUL = float2(TileUpLeft.x, -TileUpLeft.y) / float2(ScreenWidth, ScreenHeight) * 2 - float2(1, 1);
    //float2 ndcUR = float2(TileUpRight.x, -TileUpRight.y) / float2(ScreenWidth, ScreenHeight) * 2 - float2(1,1);
    //float2 ndcLL = float2(TileBottomLeft.x, -TileBottomLeft.y) / float2(ScreenWidth, ScreenHeight) * 2 - float2(1,1);
    //float2 ndcLR = float2(TileBottomRight.x, -TileBottomRight.y) / float2(ScreenWidth, ScreenHeight) * 2 - float2(1,1);
    
    // 이건 오픈지엘 기준
    //float2 screenSize = float2(ScreenWidth, ScreenHeight);

    //// 반드시 float 변환 후 부호 반전할 것!
    //float2 ndcUL = float2((float) TileUpLeft.x, -(float) TileUpLeft.y) / screenSize * 2.0f - float2(1.0f, 1.0f);
    //float2 ndcUR = float2((float) TileUpRight.x, -(float) TileUpRight.y) / screenSize * 2.0f - float2(1.0f, 1.0f);
    //float2 ndcLL = float2((float) TileBottomLeft.x, -(float) TileBottomLeft.y) / screenSize * 2.0f - float2(1.0f, 1.0f);
    //float2 ndcLR = float2((float) TileBottomRight.x, -(float) TileBottomRight.y) / screenSize * 2.0f - float2(1.0f, 1.0f);

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
    TileLightIndicesListCS[tileIndex].LightIndices[0] = 0;

    if (IsTileCenterInsideFrustum(float3(0, 0, 0), Planes))
    //if(Planes[0].y + 1 == Planes[0].y)
    {
        TileLightIndicesListCS[tileIndex].LightIndices[1] = 8;
    }
    else
    {
        TileLightIndicesListCS[tileIndex].LightIndices[1] = 0;
    }
    
    TileLightIndicesListCS[tileIndex].LightIndices[2] = Planes[0].z + 1;
    //TileLightIndicesListCS[tileIndex].LightIndices[2] = 1;
    
    // ---------------------
// Tile 좌표 정보 (Screen space 기준, 픽셀 단위)
// ---------------------
    TileLightIndicesListCS[tileIndex].LightIndices[0] = TileUpLeft.x;
    TileLightIndicesListCS[tileIndex].LightIndices[1] = TileUpLeft.y;
    TileLightIndicesListCS[tileIndex].LightIndices[2] = Right;
    TileLightIndicesListCS[tileIndex].LightIndices[3] = Bottom;

    TileLightIndicesListCS[tileIndex].LightIndices[4] = TileUpRight.x;
    TileLightIndicesListCS[tileIndex].LightIndices[5] = TileUpRight.y;
    TileLightIndicesListCS[tileIndex].LightIndices[6] = TileBottomLeft.x;
    TileLightIndicesListCS[tileIndex].LightIndices[7] = TileBottomLeft.y;
    TileLightIndicesListCS[tileIndex].LightIndices[8] = TileBottomRight.x;
    TileLightIndicesListCS[tileIndex].LightIndices[9] = TileBottomRight.y;

// ---------------------
// NDC X 값들 (범위: -1 ~ 1)
// ---------------------
    TileLightIndicesListCS[tileIndex].LightIndices[10] = ndcUL.x;
    TileLightIndicesListCS[tileIndex].LightIndices[11] = ndcUR.x;
    TileLightIndicesListCS[tileIndex].LightIndices[12] = ndcLL.x;
    TileLightIndicesListCS[tileIndex].LightIndices[13] = ndcLR.x;

// ---------------------
// NDC Y 값들 (범위: -1 ~ 1)
// ---------------------
    TileLightIndicesListCS[tileIndex].LightIndices[14] = ndcUL.y;
    TileLightIndicesListCS[tileIndex].LightIndices[15] = ndcUR.y;
    TileLightIndicesListCS[tileIndex].LightIndices[16] = ndcLL.y;
    TileLightIndicesListCS[tileIndex].LightIndices[17] = ndcLR.y;
    
    TileLightIndicesListCS[tileIndex].LightIndices[18] = (float) TileUpLeft.y;
    TileLightIndicesListCS[tileIndex].LightIndices[19] = -(float) TileUpLeft.y;
    TileLightIndicesListCS[tileIndex].LightIndices[20] = -(float) TileUpLeft.y / ScreenHeight;
    TileLightIndicesListCS[tileIndex].LightIndices[21] = ndcUL.y;

    
    //TileLightIndicesListCS[tileIndex].LightIndices[5] = DTid.x;
    //TileLightIndicesListCS[tileIndex].LightIndices[6] = DTid.y;
    //TileLightIndicesListCS[tileIndex].LightIndices[7] = 0;
    //TileLightIndicesListCS[tileIndex].LightIndices[8] = 0;
    
    //TileLightIndicesListCS[tileIndex].LightIndices[5-4] = TileUpLeft.x;
    //TileLightIndicesListCS[tileIndex].LightIndices[6-4] = TileUpLeft.y;
    //TileLightIndicesListCS[tileIndex].LightIndices[7-4] = Right;
    //TileLightIndicesListCS[tileIndex].LightIndices[8-4] = Bottom;
    
    
    //TileLightIndicesListCS[tileIndex].LightIndices[5] = ndcUL.x;
    //TileLightIndicesListCS[tileIndex].LightIndices[6] = ndcUR.x;
    //TileLightIndicesListCS[tileIndex].LightIndices[7] = ndcLL.x;
    //TileLightIndicesListCS[tileIndex].LightIndices[8] = ndcLR.x;
        
    //TileLightIndicesListCS[tileIndex].LightIndices[5 + 4] = ndcUL.y;
    //TileLightIndicesListCS[tileIndex].LightIndices[6 + 4] = ndcUR.y;
    //TileLightIndicesListCS[tileIndex].LightIndices[7 + 4] = ndcLL.y;
    //TileLightIndicesListCS[tileIndex].LightIndices[8 + 4] = ndcLR.y;
    
    //for (int k = 0; k < 6; k++)
    //{
    //    TileLightIndicesListCS[tileIndex].LightIndices[6 + 3*k + 0] = Planes[k].x * 10;
    //    TileLightIndicesListCS[tileIndex].LightIndices[6 + 3*k + 1] = Planes[k].y * 10;
    //    TileLightIndicesListCS[tileIndex].LightIndices[6 + 3*k + 2] = Planes[k].z * 10;
    //}
    
    
    
    return;
    
        int NumIntersection = 0;
    for (uint i = 0; i < 31; ++i)
    {
        if (PointLightBufferList[i].Radius <= 0)
        {
            continue;
        }
        if (SphereInFrustum(PointLightBufferList[i].Position, PointLightBufferList[i].Radius, Planes))
        {
            NumIntersection++;
        }
    }
    
    TileLightIndicesListCS[tileIndex].LightCount = NumIntersection;
    TileLightIndicesListCS[tileIndex].LightIndices[0] = 0; //RayOnZplane.x / 1;
    TileLightIndicesListCS[tileIndex].LightIndices[1] = NumIntersection; //RayOnZplane.y / 1;
    //TileLightIndicesListCS[tileIndex].LightIndices[2] = RayOnZplane.z / 1;
    
    
    return;
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    ///////////////////////////////////////////////
    //// tile의 center에서 ray
    //// --- 1. 타일 중심 NDC 계산 ---
    //float2 tileCenterScreen = TileUpLeft + TileSize * 0.5f;
    //float2 ndcCenter = (tileCenterScreen / float2(ScreenWidth, ScreenHeight)) * 2 - 1;
    //ndcCenter.y *= -1; // 좌표계 뒤집기

    //// --- 2. clip space → view space ---
    //float4 clipNear = float4(ndcCenter, 0, 1); // Near plane
    //float4 clipFar = float4(ndcCenter, 1, 1); // Far plane

    //float4 viewNear4 = mul(clipNear, ProjInv);
    //float4 viewFar4 = mul(clipFar, ProjInv);
    
    
    //viewNear4 = mul(viewNear4, ViewMatrixInv);
    //viewFar4 = mul(viewFar4, ViewMatrixInv);

    //float3 viewNear = viewNear4.xyz / viewNear4.w;
    //float3 viewFar = viewFar4.xyz / viewFar4.w;
    
    //float3 rayOrigin = viewNear;
    //float3 rayDir = normalize(viewFar - viewNear);
    
    //float t = -rayOrigin.z / rayDir.z;
    
    //float3 RayOnZplane = rayOrigin + rayDir * t;
    
    //bool a = false;
    //int NumIntersection = 0;
    //for (uint i = 0; i < 31; ++i)
    //{
    //    if (PointLightBufferList[i].Radius <= 0)
    //    {
    //        continue;
    //    }
    //    if (IntersectRaySphere(rayOrigin, rayDir, PointLightBufferList[i].Position, PointLightBufferList[i].Radius))
    //    {
    //        a = true;
    //        NumIntersection++;
    //    }
    //}
    ////bool b = IntersectRaySphere(rayOrigin, rayDir, PointLightBufferList[0].Position, PointLightBufferList[0].Radius);
    
    //TileLightIndicesListCS[tileIndex].LightCount = rayDir.x * 100000;
    //TileLightIndicesListCS[tileIndex].LightIndices[0] = 0; //RayOnZplane.x / 1;
    //TileLightIndicesListCS[tileIndex].LightIndices[1] = NumIntersection; //RayOnZplane.y / 1;
    //TileLightIndicesListCS[tileIndex].LightIndices[2] = RayOnZplane.z / 1;
    //TileLightIndicesListCS[tileIndex].LightIndices[3] = clipNear.x * 10000;
    //TileLightIndicesListCS[tileIndex].LightIndices[4] = clipNear.y * 10000;
    //TileLightIndicesListCS[tileIndex].LightIndices[5] = clipNear.z * 10000;
    //TileLightIndicesListCS[tileIndex].LightIndices[6] = clipNear.x * 10000;
    //TileLightIndicesListCS[tileIndex].LightIndices[7] = clipNear.x * 10000;
    //TileLightIndicesListCS[tileIndex].LightIndices[8] = clipNear.x * 10000;
    
    //if (a)
    //{
    //    //TileLightIndicesListCS[tileIndex].LightIndices[2] = 1;
    //}
    //return;
    
}






































//    // --- 임시 인덱스 버퍼 초기화 ---
//    uint TempIndices[31];
//    for (int i = 0; i < 31; ++i)
//    {
//        TempIndices[i] = 0;
//    }
//    uint TempCount = 0;

//    uint maxLightCount;
//    uint stride;
//    PointLightBufferList.GetDimensions(maxLightCount, stride);

//    for (uint i = 0; i < maxLightCount; ++i)
//    {
//        if (TempCount >= 31)
//            break;

//        FPointLightBuffer light = PointLightBufferList[i];
//        if (light.Radius <= 0)
//            continue;

//        float4 viewLight4 = mul(ViewMatrix, float4(light.Position, 1.0f));
//        float3 lightViewPos = viewLight4.xyz;

//        float3 oc = rayOrigin - lightViewPos;
//        float b = dot(oc, rayDir);
//        float c = dot(oc, oc) - light.Radius * light.Radius;
//        float h = b * b - c;

//        if (h >= 0.0f)
//        {
//            TempIndices[TempCount++] = i;
//        }
//    }

//    // --- 결과 복사 ---
//    FTileLightIndex result;
//    result.LightCount = TempCount;

//    for (int j = 0; j < 31; ++j)
//    {
//        result.LightIndices[j] = (j < TempCount) ? TempIndices[j] : 0;
//    }

//    TileLightIndicesListCS[tileIndex] = result;

//}



    //float2 NDC[4]; // UL, UR, LL, LR
    //NDC[0] = float2(TileUpLeft.x / ScreenWidth, -TileUpLeft.y / ScreenHeight) * 2 + float2(-1,1);
    //NDC[1] = float2(TileUpRight.x / ScreenWidth, -TileUpRight.y / ScreenHeight) * 2 + float2(-1, 1);
    //NDC[2] = float2(TileBottomLeft.x / ScreenWidth, -TileBottomLeft.y / ScreenHeight) * 2 + float2(-1, 1);
    //NDC[3] = float2(TileBottomRight.x / ScreenWidth, -TileBottomRight.y / ScreenHeight) * 2 + float2(-1, 1);
    
    //float2 tileCenterNDC = ((NDC[0] + NDC[3]) * 0.5f);
    //float4 clipNear = float4(tileCenterNDC, 0, 1); // near plane z=0
    //float4 clipFar = float4(tileCenterNDC, 1, 1); // far plane z=1

    //float3 viewNear = mul(ProjInv, clipNear).xyz;
    //viewNear /= mul(ProjInv, clipNear).w;

    //float3 viewFar = mul(ProjInv, clipFar).xyz;
    //viewFar /= mul(ProjInv, clipFar).w;

    //float3 rayDir = normalize(viewFar - viewNear); // 정규화된 ray 방향
    //float3 rayOrigin = viewNear;
    //uint TempIndices[31];
    //for (int i = 0; i < 31; ++i)
    //{
    //    TempIndices[i] = 0;
    //}
    //uint TempCount = 0;
    
    //for (uint i = 0; i < MAX_NUM_GLOBAL_LIGHT; ++i)
    //{
    //    FPointLightBuffer light = PointLightBufferList[i];
    //    // TArray.Reserve에서 해주면 -431602080.0000이 들어감
    //    if (light.Radius <= 0)
    //    {
    //        continue;
    //    }
    //    //TempCount++;
    //    float4 lightViewPos4 = mul(ViewMatrix, float4(light.Position, 1.0));
    //    float3 lightViewPos = lightViewPos4.xyz;

    //    if (IntersectRaySphere(rayOrigin, rayDir, lightViewPos, light.Radius))
    //    {
    //// 조명이 이 타일에 영향을 줌
    //        if (TempCount < 31)
    //        {
    //            TempIndices[TempCount++] = i;
    //        }
    //    }
    //}
    
    //for (int j = 0; j < 31; ++j)
    //{
    //    TileLightIndicesListCS[tileIndex].LightIndices[j] = TempIndices[j];
    //}
    //TileLightIndicesListCS[tileIndex].LightCount = TempCount;
    
    //TileLightIndicesListCS[tileIndex].LightCount = -ViewMatrix[3][0] / 256;
    //return;
    
    
    //float3 ViewPos[4];
    //for (int i = 0; i < 4; ++i)
    //{
    //    float4 clip = float4(NDC[i], 1, 1); // z=1 (far plane)
    //    float4 view = mul(ProjInv, clip);
    //    ViewPos[i] = view.xyz / view.w;
    //}
    
    //// 3. 6개 평면 구하기
    //float4 FrustumPlanes[6];
    //ComputeFrustumPlanes(ViewPos, FrustumPlanes); // 이건 직접 구현해야 함
    
    //FTileLightIndex tileLightData;
    //tileLightData.LightCount = 0;
    //for (int i = 0; i < 31; ++i)
    //{
    //    tileLightData.LightIndices[i] = 0;
    //}

    //uint maxLightCount;
    //uint lightCount = 0;
    //uint stride;
    //PointLightBufferList.GetDimensions(maxLightCount, stride);
    
    //uint TempIndices[31];
    //for (int i = 0; i < 31; ++i)
    //{
    //    TempIndices[i] = 0;
    //}
    //uint TempCount = 0;
    //for (uint i = 0; i < MAX_NUM_GLOBAL_LIGHT; ++i)
    //{
    //    FPointLightBuffer light = PointLightBufferList[i];
    //    // TArray.Reserve에서 해주면 -431602080.0000이 들어감
    //    if (light.Radius <= 0)
    //    {
    //        continue;
    //    }
    //    // Light를 View space로 변환
    //    float4 viewPos = mul(ViewMatrix, float4(light.Position, 1.0f));
    //    float3 viewSpaceLightPos = viewPos.xyz;
    //    // Frustum과의 교차 판정
    //    if (SphereInFrustum(viewSpaceLightPos, light.Radius, FrustumPlanes))
    //    {
    //        if (TempCount < 31)
    //        {
    //            TempIndices[TempCount] = i;
    //            TempCount++;
    //            //tileLightData.LightIndices[lightCount] = i;
    //            //lightCount++;
    //        }
    //    }
    //}
    
    //for (int j = 0; j < 31; ++j)
    //{
    //    TileLightIndicesListCS[tileIndex].LightIndices[j] = TempIndices[j];
    //}
    //TileLightIndicesListCS[tileIndex].LightCount = PointLightBufferList[0].Radius;
    //TileLightIndicesListCS[tileIndex].LightCount += PointLightBufferList[1].Radius;
    //TileLightIndicesListCS[tileIndex].LightCount += PointLightBufferList[2].Radius;
    ////TileLightIndicesListCS[tileIndex].LightCount += PointLightBufferList[3].Radius;
    
    ////TileLightIndicesListCS[tileIndex].LightCount *= TileLightIndicesListCS[tileIndex].LightCount;

    ////TileLightIndicesListCS[DTid.x] = tileLightData;
    ////tileLightData.LightCount = 123;
//}

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
    
    return float4(
    TileLightIndicesListPS[tileIndex].LightIndices[0]/4.f,
    TileLightIndicesListPS[tileIndex].LightIndices[1]/4.f,
    TileLightIndicesListPS[tileIndex].LightIndices[2]/4.f,
    0.5f);
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
