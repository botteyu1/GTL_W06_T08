float4 ComputePlane(float3 a, float3 b, float3 c)
{
    float3 ab = b - a;
    float3 ac = c - a;
    // input은 normal 방향기준 시계방향
    // 외적 방향이 반대로 되어있음
    //float3 normal = normalize(cross(ab, ac));
    float3 normal = -normalize(cross(ab, ac));
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

    // Plane order: left, right, top, bottom, near, far
    planes[0] = ComputePlane(world[0], world[2], world[6]); // left
    planes[1] = ComputePlane(world[3], world[1], world[7]); // right
    planes[2] = ComputePlane(world[1], world[0], world[5]); // top
    planes[3] = ComputePlane(world[2], world[3], world[6]); // bottom
    planes[4] = ComputePlane(world[0], world[1], world[2]); // near
    planes[5] = ComputePlane(world[6], world[7], world[5]); // far
}

bool SphereFrustumPlaneIntersection(float3 center, float radius, float4 planes[6])
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
    float3 OriginToCenter = sphereCenter - rayOrigin;
    
    //if (dot(OriginToCenter, OriginToCenter) < radius * radius)
    //{
    //    return true;
    //}
    
    //if (dot(rayDir, OriginToCenter) < 0)
    //{
    //    return false;
    //}
    
    
    float3 oc = rayOrigin - sphereCenter;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0 * a * c;
    return discriminant >= 0.0;
}


bool IntersectAABBSphere(float3 center, float radius, in float3 corners[8])
{
    static const float FLOAT_MAX = 3.402823e+38f;
    float3 MinCorner = float3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
    float3 MaxCorner = -float3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
    
    for (int i = 0; i < 8; i++)
    {
        MinCorner = min(MinCorner, corners[i]);
        MaxCorner = max(MaxCorner, corners[i]);
    }
    
    float3 BoxCenter = (MinCorner + MaxCorner) / 2.f;
    float3 BoxExtent = (MaxCorner - MinCorner) / 2.f;
    
    float3 Delta = max(0, abs(BoxCenter - center) - BoxExtent);
    float DistSq = dot(Delta, Delta);
    
    return DistSq <= radius * radius;
    
    return true;
}

// RGBA를 하나의 uint로 pack
uint PackColor8(float4 color)
{
    uint r = (uint) (saturate(color.r) * 255.0f);
    uint g = (uint) (saturate(color.g) * 255.0f);
    uint b = (uint) (saturate(color.b) * 255.0f);
    uint a = (uint) (saturate(color.a) * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// uint에서 float4(RGBA)로 unpack
float4 UnpackColor8(uint packed)
{
    float b = (float) (packed & 0xFF) / 255.0f;
    float g = (float) ((packed >> 8) & 0xFF) / 255.0f;
    float r = (float) ((packed >> 16) & 0xFF) / 255.0f;
    float a = (float) ((packed >> 24) & 0xFF) / 255.0f;
    return float4(r, g, b, a);
}


float3 HeatmapColor(float value, float minValue, float maxValue)
{
    // 정규화
    float t = saturate((value - minValue) / (maxValue - minValue));

    // t: 0 ~ 1 사이 값을 기반으로 색상 결정
    float3 color;

    if (t < 0.25)
    {
        // 파랑 → 청록
        float localT = t / 0.25;
        color = lerp(float3(0.0, 0.0, 1.0), float3(0.0, 1.0, 1.0), localT);
    }
    else if (t < 0.5)
    {
        // 청록 → 초록
        float localT = (t - 0.25) / 0.25;
        color = lerp(float3(0.0, 1.0, 1.0), float3(0.0, 1.0, 0.0), localT);
    }
    else if (t < 0.75)
    {
        // 초록 → 노랑
        float localT = (t - 0.5) / 0.25;
        color = lerp(float3(0.0, 1.0, 0.0), float3(1.0, 1.0, 0.0), localT);
    }
    else
    {
        // 노랑 → 빨강
        float localT = (t - 0.75) / 0.25;
        color = lerp(float3(1.0, 1.0, 0.0), float3(1.0, 0.0, 0.0), localT);
    }

    return color;
}




//bool SphereFrustumCornerIntersection(float3 center, float radius, float3 corners[8])
//{
//    for (int i = 0; i < 8; ++i)
//    {
//        float3 delta = center - corners[i];
//        float distSq = dot(delta, delta);
//        float radiusSq = radius * radius;
//        if (distSq < radiusSq)
//        {
//            return true;
//        }
//    }
//    return false;
//}
