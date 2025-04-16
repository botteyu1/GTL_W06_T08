cbuffer ConstantBufferScene : register(b0)
{
    row_major matrix ViewMatrix;
    row_major matrix ProjMatrix;
    float3 CameraPos;
    float3 CameraLookAt;
};

cbuffer ConstantBufferMesh : register(b6)
{
    row_major matrix ModelMatrix;
    row_major matrix ModelInvTransMatrix;
    //FMaterialConstants Material;
    uint IsSelectedMesh;
};

struct AABBData
{
    float3 Position;
    float3 Extent;
};
cbuffer ConstantBufferDebugAABB : register(b13)
{
    AABBData DataAABB[8];
}

struct SphereData
{
    float3 Position;
    float Radius;
    float4 Color;
};
cbuffer ConstantBufferDebugSphere : register(b13)
{
    SphereData DataSphere[8];
}

struct ConeData
{
    float3 ApexPosiiton;
    float Radius;
    float3 Direction;
    float Angle;
    float4 Color;
};
cbuffer ConstantBufferDebugCone : register(b13)
{
    ConeData DataCone[8];
}

cbuffer ConstantBufferDebugGrid : register(b13)
{
    float GridSpacing;
    int GridCount; // 총 grid 라인 수
    float3 GridOrigin; // Grid의 중심
    float Padding;
};

struct IconData
{
    float3 IconPosition;
    float IconScale;
    float4 IconColor;
};
cbuffer ConstantBufferDebugIcon : register(b13)
{
    IconData IconDatas[1024];
}

cbuffer ConstantBufferDebugArrow : register(b13)
{
    float3 ArrowPosition;
    float ArrowScaleXYZ;
    float3 ArrowDirection;
    float ArrowScaleZ;
    float4 ArrowColor;
}
