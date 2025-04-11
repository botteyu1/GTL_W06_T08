// light.hlsl

#define MAX_LIGHTS 16
#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3

struct LIGHT
{
    float3 m_cDiffuse;
    float pad2;

    float3 m_cSpecular;
    float pad3;

    float3 m_vPosition;
    float m_fFalloff; // 스팟라이트의 감쇠 인자

    float3 m_vDirection;
    float pad4;

    float m_fAttenuation; // 거리 기반 감쇠 계수
    int m_bEnable;
    int m_nType;
    float m_fIntensity; // 광원 강도
    
    float m_fAttRadius; // 감쇠 반경 (Attenuation Radius)
    float3 LightPad;
};


struct Ambient
{
    float3 AmbientColor;
    float Intensity;
    
    int bVisible;
    float3 Pad0;
};

struct Directional
{
    float3 DirectionalColor;
    float Intensity;

    float3 Direction;
    float Pad0;

    int bVisible;
    float3 Pad1;
};

struct PointLight
{
    float3 PointColor;
    float Intensity;

    float3 Position;
    float Attenuation;

    float Falloff;
    float3 Pad0;

    int bVisible;
    float3 Pad1;
};

struct SpotLight
{
    float3 Color;
    float Intensity;

    float AttenuationRadius;
    float InnerConeAngle;
    float OuterConeAngle;
    float Falloff;

    float3 Direction;
    float Pad0;

    float3 Position;
    int bVisible;
};

cbuffer cbLights : register(b2)
{
    Ambient gAmbient;

    Directional gDirectional;

    PointLight gPoint[MAX_POINT_LIGHTS];

    SpotLight gSpot[MAX_SPOT_LIGHTS];

    int gNumPointLights;
    float3 Pad0;

    int gNumSpotLights;
    float3 Pad1;

    // Legacy
    // LIGHT gLights[MAX_LIGHTS];
    // float4 gcGlobalAmbientLight;
    // int gnLights;
    // float3 padCB;
};

float4 CalculateSpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gSpot[nIndex].Position - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gSpot[nIndex].AttenuationRadius)
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
    }
    
    float fSpotFactor = pow(max(dot(-vToLight, gSpot[nIndex].Direction), 0.0f), gSpot[nIndex].Falloff);
    float fAttenuationFactor = 1.0f / (1.0f + gSpot[nIndex].AttenuationRadius * fDistance * fDistance);
    
    float3 lit = (gAmbient.AmbientColor * Material.AmbientColor.rgb) +
                 (gSpot[nIndex].Color * fDiffuseFactor * Material.DiffuseColor) +
                 (gSpot[nIndex].Color * fSpecularFactor * Material.SpecularColor);

    // intensity와 attenuation factor, spot factor를 곱하여 최종 색상 계산
    return float4(lit * fAttenuationFactor * fSpotFactor * gSpot[nIndex].Intensity, 1.0f);
}

float4 CalculatePointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gPoint[nIndex].Position - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gPoint[nIndex].Attenuation)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
    }

    float fAttenuationFactor = 1.0f / (1.0f + gPoint[nIndex].Attenuation * fDistance * fDistance);
   
    float3 lit = (gAmbient.AmbientColor * Material.AmbientColor.rgb) +
                 (gPoint[nIndex].PointColor * fDiffuseFactor * Material.DiffuseColor) +
                 (gPoint[nIndex].PointColor * fSpecularFactor * Material.SpecularColor);

    return float4(lit * fAttenuationFactor * gPoint[nIndex].Intensity, 1.0f);
}

float3 CalculateDirectionalLight(float3 vPosition, float3 vNormal, float3 BaseColor)
{
    float3 LightDirection = normalize(-gDirectional.Direction);
    float Diff = max(dot(vNormal, LightDirection), 0.0f);

    float3 Diffuse = gDirectional.DirectionalColor * Diff * BaseColor * gDirectional.Intensity;

    float3 ReflectionDirection = reflect(-LightDirection, vNormal);

    float3 View = normalize(CameraPosition - vPosition);
    float Spec = pow(max(dot(View, ReflectionDirection), 0.0f), Material.SpecularScalar);
    float3 Specular = gDirectional.DirectionalColor * Material.SpecularColor * Spec * gDirectional.Intensity;

    return Diffuse * Specular;
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    // [unroll(MAX_LIGHTS)]
    // for (int i = 0; i < gnLights; i++)
    // {
    //     if (gLights[i].m_bEnable)
    //     {
    //         if (gLights[i].m_nType == POINT_LIGHT)
    //         {
    //             cColor += PointLight(i, vPosition, vNormal);
    //         }
    //         else if (gLights[i].m_nType == SPOT_LIGHT)
    //         {
    //             cColor += SpotLight(i, vPosition, vNormal);
    //         }
    //     }
    // }

    [unroll(MAX_POINT_LIGHTS)]
    for (int i = 0; i < gNumPointLights; i++)
    {
        if (gPoint[i].bVisible)
        {
            cColor += CalculatePointLight(i, vPosition, vNormal);

        }
    }

    [unroll(MAX_SPOT_LIGHTS)]
    for (int i = 0; i < gNumSpotLights; i++)
    {
        if (gSpot[i].bVisible)
        {
            cColor += CalculateSpotLight(i, vPosition, vNormal);
        }
    }
    
    // 전역 환경광 추가
    cColor += float4(gAmbient.AmbientColor, 1.0f);

    // 전역 디렉셔널 추가
    cColor += float4( gDirectional.DirectionalColor /*CalculateDirectionalLight(vPosition, vNormal, gDirectional.DirectionalColor)*/, 1.0f);
    cColor.w = 1;
    
    return cColor;
}





