// 광원으로부터 특정 지점까지 얼마나 빛이 전파되는지 계산

// !!! 예시 !!!
// 광원 타입에 따른 effective light direction과 attenuation 계산
//void ComputeLightContribution(in SurfaceInfo surf, in LightInfo light, out float3 effectiveL, out float attenuation)
//{
//    if (light.Type == DIRECTIONAL)
//    {
//        // Directional Light는 무한대 광원: 방향만 고려하며, 감쇠 없음
//        effectiveL = normalize(-light.Direction);
//        attenuation = 1.0f;
//    }
//    else if (light.Type == SPOT)
//    {
//        // Spot Light: 위치와 방향이 있으며, 거리와 각도 감쇠 적용
//        effectiveL = normalize(light.Position - surf.WorldPos);
        
//        // 각도 감쇠: 광원 중심과 픽셀의 각도를 비교
//        float3 lightToPixel = normalize(surf.WorldPos - light.Position);
//        float spotCos = dot(-light.Direction, lightToPixel);
//        float angleFactor = saturate((spotCos - light.OuterConeCos) / (light.InnerConeCos - light.OuterConeCos));

//        // 거리 감쇠: 거리 기반 선형 감쇠 (다양한 감쇠 모형 적용 가능)
//        float distance = length(light.Position - surf.WorldPos);
//        float distanceAtten = saturate(1.0f - distance / light.Range);

//        attenuation = angleFactor * distanceAtten;
//    }
//    else
//    {
//        // 기본값 (예외 처리)
//        effectiveL = float3(0, 0, 0);
//        attenuation = 0.0f;
//    }
//}

// !!! 예시 !!!
//float3 ComputeGouraudAtVertex(float3 normal, float3 viewDir, float3 worldPos, LightInfo light, float shininess)
//{
//    float3 lightDir;
//    float attenuation;
//    SurfaceInfo surf;
//    surf.Normal = normalize(normal);
//    surf.ViewDir = normalize(viewDir);
//    surf.WorldPos = worldPos;

//    ComputeLightContribution(surf, light, lightDir, attenuation);

//    float3 color = ComputeBlinnPhong(surf, BlinnPhongParams(
//        lightDir,
//        light.Color * attenuation,
//        shininess
//    ));

//    return color; // 이걸 vertex shader에서 보간
//}
