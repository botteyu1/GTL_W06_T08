// UberLit.hlsl
#include "UberLitConstants.hlsli"
#include "UberLitTextures.hlsli"

VS_OUT Uber_VS(VS_IN Input)
{
    VS_OUT output;
#if LIGHTING_MODEL_GOURAUD

#elif LIGHTING_MODEL_LAMBERT

#elif LIGHTING_MODEL_PHONG

#endif
    output.position = float4(1, 1, 1, 1);
    return output;
}

float4 Uber_PS(VS_OUT Input) : SV_TARGET
{
    float4 finalPixel = DiffuseTexture.Sample(Sampler, Input.texcoord);

    // Ambient Light의 컬러를 계산
    finalPixel += float4(AmbientLight.AmbientColor, 1.f) * AmbientLight.Intensity;
    
#if LIGHTING_MODEL_GOURAUD

#elif LIGHTING_MODEL_LAMBERT
    finalPixel += CalculateAmbientLight(...);
    for(It : PointLights)
    {
        finalPixel += CalculatePointLight(...);
    }
#elif LIGHTING_MODEL_PHONG
    // Specular Reflectance
#endif
    return finalPixel;
}
