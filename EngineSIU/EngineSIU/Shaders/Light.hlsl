// light.hlsl

#define NUM_MAX_DIRLIGHT 1
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 8

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
};

struct Directional
{
    float3 DirectionalColor;
    float Intensity;

    float3 Direction;
    float Pad0;
};

struct PointLight
{
    float3 PointColor;
    float Intensity;

    float3 Position;
    float AttenuationRadius;

    float Falloff;
    float3 Pad0;
};

struct SpotLight
{
    float3 Color;
    float Intensity;

    float3 Position;
    float AttenuationRadius;

    float3 Direction;
    float Falloff;
    
    float InnerConeAngle;
    float OuterConeAngle;
    float2 pad;
};

cbuffer cbLights : register(b2)
{
    Ambient gAmbient;

    Directional gDirectional[NUM_MAX_DIRLIGHT];

    PointLight gPoint[MAX_POINT_LIGHTS];

    SpotLight gSpot[MAX_SPOT_LIGHTS];

    int gNumDirectionalLights; // =1
    int gNumPointLights;
    int gNumSpotLights;
    int pad1;
};

float4 CalculateSpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    // float3 vToLight = gSpot[nIndex].Position - vPosition;
    // float fDistance = length(vToLight);
    //
    // // 감쇠 반경을 벗어나면 기여하지 않음
    // if (fDistance > gSpot[nIndex].AttenuationRadius)
    // {
    //     return float4(0.0f, 0.0f, 0.0f, 0.0f);
    // }
    //
    // float fSpecularFactor = 0.0f;
    // vToLight /= fDistance; // 정규화
    //
    // float fDiffuseFactor = saturate(dot(vNormal, vToLight));
    //
    // if (fDiffuseFactor > 0.0f)
    // {
    //     float3 vView = normalize(CameraPosition - vPosition);
    //     float3 vHalf = normalize(vToLight + vView);
    //     fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
    // }
    //
    // float fSpotFactor = pow(max(dot(-vToLight, gSpot[nIndex].Direction), 0.0f), gSpot[nIndex].Falloff);
    // float fAttenuationFactor = 1.0f / (1.0f + gSpot[nIndex].AttenuationRadius * fDistance * fDistance);
    //
    // float3 lit = (gAmbient.AmbientColor * Material.AmbientColor.rgb) +
    //              (gSpot[nIndex].Color * fDiffuseFactor * Material.DiffuseColor) +
    //              (gSpot[nIndex].Color * fSpecularFactor * Material.SpecularColor);

    float3 Diffuse;
    float3 Specular;

    // Blinn-phong
    float3 ToLight = gSpot[nIndex].Position - vPosition;
    float Distance = length(ToLight);
    float3 LightDirection = ToLight / Distance;
    
    // 감쇠거리
    float Attenuation = saturate(1.0 - Distance / gSpot[nIndex].AttenuationRadius);
    Attenuation = pow(Attenuation, gSpot[nIndex].Falloff);

    // Spot Cone 각도
    float CosAngle = dot(gSpot[nIndex].Direction, -LightDirection);
    float Outer = cos(radians(gSpot[nIndex].OuterConeAngle / 2.0f));
    float Inner = cos(radians(gSpot[nIndex].InnerConeAngle / 2.0f));

    float ConeAttenuation = saturate((CosAngle - Outer) / (Inner - Outer));
    ConeAttenuation = pow(ConeAttenuation, gSpot[nIndex].Falloff);

    float LightScale = gSpot[nIndex].Intensity * ConeAttenuation * Attenuation;
    
    float3 ViewDirection = normalize(CameraPosition - vPosition);
    float3 HalfVector = normalize(ViewDirection + LightDirection);

    float NdotL = max(dot(vNormal, LightDirection), 0.0f);
    float NdotH = max(dot(vNormal, HalfVector), 0.0f);

    Diffuse = gSpot[nIndex].Color * NdotL * Material.DiffuseColor * LightScale;
    Specular = gSpot[nIndex].Color * pow(NdotH, Material.SpecularScalar) * LightScale;

    return float4(Diffuse + Specular, 1.0f);
}

float4 CalculatePointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 Diffuse;
    float3 Specular;

    // Blinn-phong
    float3 ToLight = gPoint[nIndex].Position - vPosition;
    float Distance = length(ToLight);
    float3 LightDirection = ToLight / Distance; // normalize

    float Attenuation = saturate(1.0f - Distance / gPoint[nIndex].AttenuationRadius);
    Attenuation = pow(Attenuation, gPoint[nIndex].Falloff);

    float3 ViewDirection = normalize(CameraPosition - vPosition);
    float3 HalfVector = normalize(ViewDirection + LightDirection);

    float NdotL = max(dot(vNormal, LightDirection), 0.0f);
    float NdotH = max(dot(vNormal, HalfVector), 0.0f);

    Diffuse = gPoint[nIndex].PointColor * NdotL * Material.DiffuseColor * Attenuation;
    Specular = gPoint[nIndex].PointColor * Material.SpecularColor * pow(NdotH, Material.SpecularScalar) * Attenuation;

    // Lambert
    //Specular = float3(0.0f, 0.0f, 0.0f);

    // Gouraud - used to vertex shader
    float3 ReflectDirection = reflect(LightDirection, vNormal);
    //Specular = gPoint[nIndex].PointColor * pow(max(dot(ReflectDirection, ViewDirection), 0.0f), Material.SpecularScalar) * Attenuation;
    
    return float4(gPoint[nIndex].Intensity * (Diffuse + Specular), 1.0f);
}

float3 CalculateDirectionalLight(float3 vPosition, float3 vNormal)
{
    float3 Specular = float3(0.0f, 0.0f, 0.0f);
    // Phong
    // float3 LightDirection = normalize(gDirectional.Direction);
    // float Diff = max(dot(-LightDirection, vNormal), 0.0f);
    //
    // float3 Diffuse = gDirectional.DirectionalColor * Diff;
    //
    // if (Diff > 0.f)
    // {
    //     float3 View = normalize(CameraPosition - vPosition);
    //     float3 ReflectionDirection = normalize(reflect(LightDirection, vNormal));
    //     float Spec = pow(max(dot(ReflectionDirection, View), 0.0f), Material.SpecularScalar);
    //     
    //     Specular = gDirectional.DirectionalColor * Material.SpecularColor * Spec;
    // }

    // Blinn-Phong
    //
    float3 LightDirection = normalize(-gDirectional[0].Direction);
    float3 View = normalize(CameraPosition - vPosition);
    float3 HalfVector = normalize(View + LightDirection);
    
    float Diff = max(dot(vNormal, LightDirection), 0.0f);
    float3 Diffuse = gDirectional[0].DirectionalColor * Diff;
    

    if (Diff > 0.0f)
    {
        float Spec = pow(max(dot(vNormal, HalfVector), 0.0f), Material.SpecularScalar);
        Specular = gDirectional[0].DirectionalColor * Spec;
    }

    // Lambert
    //
    // float3 LightDirection = normalize(gDirectional.Direction);
    // float NdotL = max(dot(vNormal, -LightDirection), 0.0f);
    //
    // float3 Diffuse = gDirectional.DirectionalColor * NdotL;

    // Gouraud
    //
    // float3 LightDirection = normalize(gDirectional.Direction);
    // float3 View = normalize(CameraPosition - vPosition);
    // float3 ReflectDirection = reflect(LightDirection, vNormal);
    //
    // float Diff = max(dot(vNormal, -LightDirection), 0.0f);
    // float Spec = pow(max(dot(ReflectDirection, View), 0.0f), Material.SpecularScalar);
    //
    // float3 Diffuse = gDirectional.DirectionalColor * Diff;
    // Specular = gDirectional.DirectionalColor * Spec;
    
    return gDirectional[0].Intensity * (Diffuse + Specular);
    
    
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    [unroll(MAX_POINT_LIGHTS)]
    for (int i = 0; i < gNumPointLights; i++)
    {
        cColor += CalculatePointLight(i, vPosition, vNormal);
    }

    [unroll(MAX_SPOT_LIGHTS)]
    for (int i = 0; i < gNumSpotLights; i++)
    {
        cColor += CalculateSpotLight(i, vPosition, vNormal);
    }
    
    // 전역 환경광 추가
    cColor += float4(gAmbient.AmbientColor, 1.0f);

    // 전역 디렉셔널 추가
    cColor += float4( CalculateDirectionalLight(vPosition, vNormal), 1.0f);
    cColor.w = 1;
    
    return cColor;
}





