// 물체 표면의 한 지점에 들어오는 빛이 어떻게 반사될것인지 계산
#include "UberLitConstants.hlsli"

// Gouraud Shading
// Per-Vertex로 값을 구함

// Lambert Shading
// Per-Pixel로 값을 구함. Diffuse만

// Blinn-Phong Shading
// Per-Pixel로 값을 구함. Specular + Diffuse

// !!! 예시 !!!
//// 공통 Blinn-Phong 계산 함수
//float3 ComputeBlinnPhong(SurfaceInfo surf, BlinnPhongParams bp)
//{
//    float NdotL = max(dot(surf.Normal, bp.LightDir), 0.0f);
    
//    // 하이라이트 계산: LightDir와 ViewDir의 중간 벡터를 이미 계산했다고 가정할 수 없으므로,
//    // 여기서는 재계산. (일반적으로 H = normalize(bp.LightDir + surf.ViewDir))
//    float3 H = normalize(bp.LightDir + surf.ViewDir);
//    float NdotH = max(dot(surf.Normal, H), 0.0f);
//    float specular = pow(NdotH, bp.Shininess);

//    return bp.LightColor * (NdotL + specular);
//}

//// 통합 Blinn-Phong 조명 함수 (Spot, Directional 모두 지원)
//float3 BlinnPhongUnified(SurfaceInfo surf, LightInfo light, float Shininess)
//{
//    float3 effectiveL;
//    float attenuation;
    
//    // 광원 타입에 따른 effective light direction과 attenuation을 계산
//    ComputeLightContribution(surf, light, effectiveL, attenuation);
    
//    // BlinnPhong 파라미터 구성
//    BlinnPhongParams bp;
//    bp.LightDir = effectiveL;
//    bp.LightColor = light.Color * attenuation;
//    bp.Shininess = Shininess;
    
//    return ComputeBlinnPhong(surf, bp);
//}
