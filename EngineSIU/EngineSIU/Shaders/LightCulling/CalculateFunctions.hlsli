float4 ComputePlane(float3 a, float3 b, float3 c)
{
    float3 ab = b - a;
    float3 ac = c - a;
    float3 normal = normalize(cross(ab, ac));
    float d = -dot(normal, a);
    return float4(normal, d);
}

void GetTileFrustumPlanes(
    float2 ndc[4], // UL, UR, LL, LR
    matrix ProjInv, // Projection Inverse (clip → view)
    matrix ViewInv, // View Inverse (view → world)
    out float4 planes[6])     // left, right, top, bottom, near, far
{
    float4 clipNear[4];
    float4 clipFar[4];
    for (int i = 0; i < 4; ++i)
    {
        clipNear[i] = float4(ndc[i], 0, 1); // Near Z = 0
        clipFar[i] = float4(ndc[i], 1, 1); // Far Z = 1
    }
    float temp;
    float3 world[8]; // 0~3 near, 4~7 far
    for (int i = 0; i < 4; ++i)
    {
        float4 viewNear = mul(clipNear[i], ProjInv);
        float4 viewFar = mul(clipFar[i], ProjInv);
        viewNear /= viewNear.w;
        viewFar /= viewFar.w;

        float4 worldNear = mul(viewNear, ViewInv);
        float4 worldFar = mul(viewFar, ViewInv);

        temp = worldNear.z;
        world[i] = worldNear.xyz;
        world[i + 4] = worldFar.xyz;
    }

    //// Plane order: left, right, top, bottom, near, far
    //planes[0] = float3(world[0].x, world[0].y, world[0].x); // left
    //planes[1] = float3(world[1].x, world[1].y, world[1].x); // right
    //planes[2] = float3(world[2].x, world[2].y, world[2].x); // top
    //planes[3] = float3(world[3].x, world[3].y, world[3].x); // bottom
    //planes[4] = float3(world[4].x, world[4].y, world[4].x); // near
    //planes[5] = float3(world[5].x, world[5].y, world[5].x); // far
    
    for (int j = 0; j < 6; j++)
    {
        planes[j] = float4(0, 0, 0, 0);

    }
    
    //// Plane order: left, right, top, bottom, near, far
    //planes[0] = ComputePlane(world[0], world[2], world[6]); // left
    //planes[1] = ComputePlane(world[3], world[1], world[7]); // right
    //planes[2] = ComputePlane(world[1], world[0], world[5]); // top
    //planes[3] = ComputePlane(world[2], world[3], world[6]); // bottom
    //planes[4] = ComputePlane(world[0], world[1], world[2]); // near
    //planes[5] = ComputePlane(world[6], world[7], world[5]); // far
}

bool SphereInFrustum(float3 center, float radius, float4 planes[6])
{
    for (int i = 0; i < 6; ++i)
    {
        float distance = dot(planes[i].xyz, center) + planes[i].w;
        // distance 음수면 plane의 뒤 
        if (distance < -radius)
        {
            return false; // 완전히 바깥
        }
    }
    return true; // 안에 있거나 걸침
}

bool IntersectRaySphere(float3 rayOrigin, float3 rayDir, float3 sphereCenter, float radius)
{
    float3 oc = rayOrigin - sphereCenter;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0 * a * c;
    return discriminant >= 0.0;
}

// Plane의 안쪽을 향하고 있는지 검사하는 함수
bool IsTileCenterInsideFrustum(float3 tileCenterWorld, float4 planes[6])
{
    bool allInside = true;

    for (int i = 0; i < 6; ++i)
    {
        float distance = dot(planes[i].xyz, tileCenterWorld) + planes[i].w;

        // 디버깅: 거리 값 확인
        // [옵션] RWBuffer나 UAV로 거리값 출력 가능

        if (distance < 0)
        {
            allInside = false;
            // [옵션] 로그 출력 or 시각화용 마킹
        }
    }
    return allInside;
}
