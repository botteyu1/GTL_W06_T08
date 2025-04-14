float4 CreatePlane(float3 a, float3 b, float3 c)
{
    float3 ab = b - a;
    float3 ac = c - a;
    float3 normal = normalize(cross(ab, ac));
    float d = -dot(normal, a);
    return float4(normal, d);
}

void ComputeFrustumPlanes(float3 ViewPos[4], out float4 FrustumPlanes[6])
{
    float3 camPos = float3(0, 0, 0); // View space 기준

    // ViewPos 순서:
    // 0 = UL, 1 = UR, 2 = LL, 3 = LR

    // 좌측 평면: (cam, UL, LL)
    FrustumPlanes[0] = CreatePlane(camPos, ViewPos[0], ViewPos[2]);
    
    // 우측 평면: (cam, LR, UR)
    FrustumPlanes[1] = CreatePlane(camPos, ViewPos[3], ViewPos[1]);
    
    // 상단 평면: (cam, UR, UL)
    FrustumPlanes[2] = CreatePlane(camPos, ViewPos[1], ViewPos[0]);
    
    // 하단 평면: (cam, LL, LR)
    FrustumPlanes[3] = CreatePlane(camPos, ViewPos[2], ViewPos[3]);
    
    // near plane: z = 0 평면, 법선이 view space에서 앞을 보는 방향
    FrustumPlanes[4] = float4(0, 0, 1, 0); // z > 0 인 부분이 inside

    // far plane
    FrustumPlanes[5] = CreatePlane(ViewPos[0], ViewPos[1], ViewPos[3]);
}

bool SphereInFrustum(float3 center, float radius, float4 planes[6])
{
    for (int i = 0; i < 6; ++i)
    {
        float distance = dot(planes[i].xyz, center) + planes[i].w;
        if (distance < -radius)
        {
            return false; // 완전히 바깥
        }
    }
    return true; // 안에 있거나 걸침
}
