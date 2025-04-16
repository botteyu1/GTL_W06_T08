// UberLit.hlsl
#include "UberLitConstants.hlsli"
#include "UberLitTextures.hlsli"
#include "UberLitLightingModels.hlsli"
#include "UberLitShadingModels.hlsli"

float LinearToSRGB(float val)
{
    float low  = 12.92 * val;
    float high = 1.055 * pow(val, 1.0 / 2.4) - 0.055;
    // linear가 임계값보다 큰지 판별 후 선형 보간
    float t = step(0.0031308, val); // linear >= 0.0031308이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float3 LinearToSRGB(float3 color)
{
    color.r = LinearToSRGB(color.r);
    color.g = LinearToSRGB(color.g);
    color.b = LinearToSRGB(color.b);
    return color;
}

VS_OUT Uber_VS(VS_IN input)
{
    VS_OUT output;
    
    output.materialIndex = input.materialIndex;
    
    float4 worldPosition = mul(float4(input.position, 1), Model);
    
    output.worldPos = worldPosition.xyz;
    
    float4 viewPosition = mul(worldPosition, View);
    
    output.position = mul(viewPosition, Projection);
    
    output.color = input.color;
  
    output.normal = normalize(mul(input.normal, (float3x3) MInverseTranspose)); // Object Space -> World Space

    float3 BiTangent = cross(input.normal, input.Tangent);
    matrix<float, 3, 3> TBN =
    {
        input.Tangent.x, input.Tangent.y, input.Tangent.z, // column 0
        BiTangent.x, BiTangent.y, BiTangent.z, // column 1
        input.normal.x, input.normal.y, input.normal.z // column 2
    };
    output.TBN = transpose(TBN);
    
    output.texcoord = input.texcoord;
#if LIGHTING_MODEL_GOURAUD
    float3 GouraudColor = ComputeGouraudShading(output.worldPos, output.normal, 1.0f,
         AmbientLight, DirectionalLights, PointLights, SpotLights,
         NumDirLights, NumPointLights, NumSpotLights);
    output.color = float4(GouraudColor, 1);
#endif
    return output;
}

PS_OUT Uber_PS(VS_OUT Input)
{
    PS_OUT Output;
    Output.UUID = UUID;
    
    float3 BaseColor = float3(1, 1, 1);
    float3 Normal = normalize(Input.normal);
    float3 MatAmbientColor = float3(0, 0, 0);
    float3 MatSpecularColor = float3(0, 0, 0);
    float3 MatAlphaColor = float3(0, 0, 0);
    float3 MatEmissiveColor = float3(0, 0, 0);
    float3 MatRoughnessColor = float3(0, 0, 0);

    float2 UV = Input.texcoord + UVOffset;
    
    // Diffuse
    if (Material.TextureFlag & (1 << 0))
    {
        BaseColor = DiffuseTexture.Sample(Sampler, UV).rgb;
    }

    // Ambient
    if (Material.TextureFlag & (1 << 1))
    {
        MatAmbientColor = AmbientTexture.Sample(Sampler, UV).rgb;
    }

    // Specular
    if (Material.TextureFlag & (1 << 2))
    {
        MatSpecularColor = SpecularTexture.Sample(Sampler, UV).rgb;
    }
    
    // Alpha
    if (Material.TextureFlag & (1 << 3))
    {
        MatAlphaColor = AlphaTexture.Sample(Sampler, UV).rgb;
    }

    // Emissive
    if (Material.TextureFlag & (1 << 4))
    {
        MatEmissiveColor = EmissiveTexture.Sample(Sampler, UV).rgb;
    }

    // Roughness
    if (Material.TextureFlag & (1 << 5))
    {
        MatRoughnessColor = RoughnessTexture.Sample(Sampler, UV).rgb;
    }
    
    // Normal (Bump)
    if (Material.TextureFlag & (1 << 6))
    {
        float3 SampledNormal = NormalTexture.Sample(Sampler, UV).rgb;
        Normal = normalize(2.f * SampledNormal - 1.f);

        /** Depreacated - processing normals by converting them to world space. */
        // Normal = normalize(mul(mul(Normal, Input.TBN), (float3x3) MInverseTranspose));
    }

    BaseColor *= Material.DiffuseColor.rgb;

    // flag 되어있으면 미리 return
    // unlit
    if (RenderFlag == 0)
    {
        Output.color = float4(BaseColor, 1);
        return Output;
    }
    // depth
    else if (RenderFlag == 2)
    {
        float z = Input.position.z / Input.position.w;
        Output.color = float4(z, z, z, 1);
        return Output;
    }
    // worldpos
    else if (RenderFlag == 3)
    {
        Output.color = float4(Input.worldPos / 100 + 0.5, 1);
        return Output;
    }
    // normal
    else if (RenderFlag == 4)
    {
        Normal = normalize(mul(mul(Normal, Input.TBN), (float3x3) MInverseTranspose));
        Output.color = float4(Normal/2 + 0.5, 1);
        return Output;
    }
    
    float3 TotalColor = float3(0, 0, 0);
    
    float3 DiffuseColor;
    float3 Direction;
    float Attenuation;

#if LIGHTING_MODEL_GOURAUD

    float3 FinalColor = BaseColor * Input.color.rgb;
    Output.color = float4(FinalColor, 1);
    return Output;
    
#elif LIGHTING_MODEL_LAMBERT
    for (int i = 0; i < NumPointLights; i++)
    {
        CalculatePointLight(PointLights[i].Position, Input.worldPos, PointLights[i].AttenuationRadius, PointLights[i].Falloff, Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction, Input.TBN);
        
        ComputeLambert(PointLights[i].Color, Direction, Normal, DiffuseColor);

        DiffuseColor *= Attenuation;
        TotalColor += PointLights[i].Intensity * DiffuseColor;
    }

    for (int i = 0; i < NumSpotLights; i++)
    {
        CalculateSpotLight(SpotLights[i].Direction, SpotLights[i].Position, Input.worldPos,
            SpotLights[i].AttenuationRadius, SpotLights[i].Falloff, SpotLights[i].InnerConeAngle, SpotLights[i].OuterConeAngle,
            Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction, Input.TBN);
        
        ComputeLambert(SpotLights[i].Color, Direction, Normal, DiffuseColor);

        DiffuseColor *= Attenuation;
        TotalColor += SpotLights[i].Intensity * DiffuseColor;
    }

    // Directional
    CalculateDirectionalLight(DirectionalLights[0].Direction, Direction);

    // Transform tangent space
    Direction = mul(Direction, Input.TBN);
    
    ComputeLambert(DirectionalLights[0].Color, Direction, Normal, DiffuseColor);

    TotalColor += DirectionalLights[0].Intensity * DiffuseColor;
#elif LIGHTING_MODEL_PHONG
    // Specular Reflectance
    float3 SpecularColor;
    
    for (int i = 0; i < NumPointLights; i++)
    {
        CalculatePointLight(PointLights[i].Position, Input.worldPos, PointLights[i].AttenuationRadius, PointLights[i].Falloff, Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction, Input.TBN);
        float3 ViewDirection = normalize(CameraPosition - Input.worldPos);
        ViewDirection = mul(ViewDirection, Input.TBN);

        ComputeBlinnPhong(PointLights[i].Color, Direction, ViewDirection, Normal, Material.SpecularScalar, DiffuseColor, SpecularColor);

        DiffuseColor *= Material.DiffuseColor * Attenuation;
        SpecularColor *= Material.SpecularColor * Attenuation;

        TotalColor += PointLights[i].Intensity * (DiffuseColor + SpecularColor);
    }

    for (int i = 0; i < NumSpotLights; i++)
    {
        CalculateSpotLight(SpotLights[i].Direction, SpotLights[i].Position, Input.worldPos,
            SpotLights[i].AttenuationRadius, SpotLights[i].Falloff, SpotLights[i].InnerConeAngle, SpotLights[i].OuterConeAngle,
            Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction,Input.TBN);
        float3 ViewDirection = normalize(CameraPosition - Input.worldPos);
        ViewDirection = mul(ViewDirection, Input.TBN);
        
        ComputeBlinnPhong(SpotLights[i].Color, Direction, ViewDirection, Normal, Material.SpecularScalar, DiffuseColor, SpecularColor);

        DiffuseColor *= SpotLights[i].Intensity * Material.DiffuseColor * Attenuation;
        SpecularColor *= SpotLights[i].Intensity * Material.SpecularColor * Attenuation;

        TotalColor += DiffuseColor + SpecularColor;
    }

    // Directional
    CalculateDirectionalLight(DirectionalLights[0].Direction, Direction);

    // Transform tangent space
    Direction = mul(Direction, Input.TBN);
    float3 ViewDirection = normalize(CameraPosition - Input.worldPos);
    ViewDirection = mul(ViewDirection, Input.TBN);
    
    ComputeBlinnPhong(DirectionalLights[0].Color, Direction, ViewDirection, Normal, Material.SpecularScalar, DiffuseColor, SpecularColor);

    DiffuseColor *= Material.DiffuseColor;
    SpecularColor *= Material.SpecularColor;

    TotalColor += DirectionalLights[0].Intensity * (DiffuseColor + SpecularColor);
#endif
    
    // Ambient Light
    TotalColor += AmbientLight.Color * AmbientLight.Intensity;

    TotalColor *= 5;
    if (RenderFlag == 5)
    {
        Output.color = float4(TotalColor, 1);
        return Output;
    }
    Output.color = float4(BaseColor * TotalColor, 1);
    
    // 선택
    if (isSelected)
    {
        Output.color += float4(0.02, 0.02, 0.02, 1);
    }
        
    return Output;
}
