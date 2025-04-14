// UberLit.hlsl
#include "UberLitConstants.hlsli"
#include "UberLitTextures.hlsli"

VS_OUT Uber_VS(VS_IN input)
{
    VS_OUT output;
    
    output.materialIndex = input.materialIndex;
    
    float4 worldPosition = mul(float4(input.position, 1), Model);
    
    output.worldPos = worldPosition.xyz;
    
    float4 viewPosition = mul(worldPosition, View);
    
    output.position = mul(viewPosition, Projection);
    
    output.color = input.color;
  
    output.normal = normalize(mul(input.normal, (float3x3) MInverseTranspose));

    float3 BiTangent = cross(input.normal, input.Tangent);
    matrix<float, 3, 3> TBN =
    {
        input.Tangent.x, input.Tangent.y, input.Tangent.z, // column 0
        BiTangent.x, BiTangent.y, BiTangent.z, // column 1
        input.normal.x, input.normal.y, input.normal.z            // column 2
    };
    output.TBN = TBN;
    
    output.texcoord = input.texcoord;
    
    return output;
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
