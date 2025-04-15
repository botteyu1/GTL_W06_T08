// 광원으로부터 특정 지점까지 얼마나 빛이 전파되는지 계산

void CalculateDirectionalLight(float3 LightDirection, out float3 EffectiveLightDirection)
{
    EffectiveLightDirection = normalize(-LightDirection);
}

void CalculatePointLight(float3 LightPosition, float3 VertexPosition, float AttenuationRadius, float Falloff, out float3 EffectiveLightDirection, out float FinalAttenuation)
{
    float3 LightDirection = LightPosition - VertexPosition;
    float Distance = length(LightDirection);

    float CalcAttenuation = saturate(1.0f - Distance / AttenuationRadius);
    CalcAttenuation = pow(CalcAttenuation, Falloff);
    EffectiveLightDirection = normalize(LightDirection);
    FinalAttenuation = CalcAttenuation;
}


/**
 * Assign the result to EffectiveLightDirection & FinalAttenuation.
 *
 * EffectiveLightDirection is not normalized.
 * 
 * @param SpotLightDirection
 * @param LightPosition
 * @param VertexPosition
 * @param AttenuationRadius
 * @param Falloff 
 * @param InnerConeAngle 
 * @param OuterConeAngle 
 * @param EffectiveLightDirection 
 * @param FinalAttenuation
 */
void CalculateSpotLight(float3 SpotLightDirection, float3 LightPosition, float3 VertexPosition,
                        float AttenuationRadius, float Falloff, float InnerConeAngle, float OuterConeAngle,
                        out float3 EffectiveLightDirection, out float FinalAttenuation)
{
    float3 LightDirection = LightPosition - VertexPosition;
    float Distance = length(LightDirection);
    float3 LightDirectionNormalized = LightDirection / Distance;
    
    // 감쇠거리
    float Attenuation = saturate(1.0 - Distance / AttenuationRadius);
    Attenuation = pow(Attenuation, Falloff);

    // Spot Cone 각도
    float CosAngle = dot(SpotLightDirection, -LightDirectionNormalized);
    float Outer = cos(radians(OuterConeAngle));
    float Inner = cos(radians(InnerConeAngle));

    float ConeAttenuation = saturate((CosAngle - Outer) / (Inner - Outer));
    ConeAttenuation = pow(ConeAttenuation, Falloff);

    EffectiveLightDirection = normalize(LightDirection);
    FinalAttenuation = (CosAngle < 0.0f) ? 0.0 : Attenuation * ConeAttenuation;
}
