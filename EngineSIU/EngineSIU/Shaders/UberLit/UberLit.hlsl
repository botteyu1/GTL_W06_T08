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

    float3 MatDiffuseColor = float3(0, 0, 0);
    float3 MatSpecularColor = float3(0, 0, 0);

    // Diffuse
    if (Material.TextureFlag & (1 << 0))
    {
        float2 UV = input.texcoord + UVOffset;
        float4 TexLevel = DiffuseTexture.SampleLevel(Sampler, UV, 0.0f);
        MatDiffuseColor = TexLevel.rgb;
    }
    else
    {
        MatDiffuseColor = Material.DiffuseColor.rgb;
    }

    // Specular
    if (Material.TextureFlag & (1 << 2))
    {
        float2 UV = input.texcoord + UVOffset;
        float4 TexLevel = DiffuseTexture.SampleLevel(Sampler, UV, 0.0f);
        MatSpecularColor = TexLevel.rgb;
    }
    else
    {
        MatSpecularColor = Material.SpecularColor.rgb;
    }
    
    float3 GouraudColor = ComputeGouraudShading(worldPosition.xyz, output.normal, Material.SpecularScalar,
         MatDiffuseColor, MatSpecularColor,
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
    
    /** GOURAUD */
#if LIGHTING_MODEL_GOURAUD
    Output.color = Input.color;
    return Output;
#endif

    float3 MatDiffuseColor;
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
        MatDiffuseColor = DiffuseTexture.Sample(Sampler, UV).rgb;
    }
    else
    {
        MatDiffuseColor = Material.DiffuseColor.rgb;
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
    else
    {
        MatSpecularColor = Material.SpecularColor.rgb;
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
    else
    {
        // If not include bump map, normal transforms tangent space.
        Normal = normalize(mul(Input.normal, Input.TBN));
    }

    // flag 되어있으면 미리 return
    // unlit
    if (RenderFlag == 0)
    {
        Output.color = float4(MatDiffuseColor, 1);
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
    
    float3 TotalDiffuse = float3(0, 0, 0);
    float3 TotalSpecular = float3(0, 0, 0);

    float3 Direction = float3(0,0,0);
    float Attenuation = 0.0f;

    float3 LightDiffuseColor = float3(0, 0, 0);
    float3 LightSpecularColor = float3(0, 0, 0);
    
    // Ambient Light
    TotalColor += AmbientLight.Color * AmbientLight.Intensity * MatDiffuseColor;

    // Light Culling Data 수집
    // 화면 좌표 기반 타일 인덱스 계산 예시
    //float2 NDCCoord = Input.position.xy / Input.position.w;
    //int2 pixelCoord = (NDCCoord + 1.f) * float2(ScreenInfo.ScreenWidth, ScreenInfo.ScreenHeight) * float2(0.5, -0.5);
    int2 pixelCoord = Input.position;
    int2 tileCoord = pixelCoord / ScreenInfo.TileSize;
    uint tileIndex = tileCoord.y * ScreenInfo.NumTileWidth + tileCoord.x;

    FTileLightIndex tileLightInfo = TileLightIndicesList[tileIndex];
    
    uint CulledLightNum = tileLightInfo.LightCount;
    uint CulledLightIndex[MAX_NUM_INDICES_PER_TILE] = tileLightInfo.LightIndices;

    
#if LIGHTING_MODEL_LAMBERT
    for (int i = 0; i < CulledLightNum; i++)
    {
        FPointLight PointLightFromBuffer = PointLightBufferList[CulledLightIndex[i]];
        CalculatePointLight(PointLightFromBuffer.Position, Input.worldPos, PointLightFromBuffer.AttenuationRadius, PointLightFromBuffer.Falloff, Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction, Input.TBN);
        
        ComputeLambert(PointLightFromBuffer.Color, Direction, Normal, LightDiffuseColor);

        TotalDiffuse += LightDiffuseColor * Attenuation * PointLightFromBuffer.Intensity;
    }

    for (int i = 0; i < NumSpotLights; i++)
    {
        CalculateSpotLight(SpotLights[i].Direction, SpotLights[i].Position, Input.worldPos,
            SpotLights[i].AttenuationRadius, SpotLights[i].Falloff, SpotLights[i].InnerConeAngle, SpotLights[i].OuterConeAngle,
            Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction, Input.TBN);
        
        ComputeLambert(SpotLights[i].Color, Direction, Normal, LightDiffuseColor);

        TotalDiffuse += LightDiffuseColor * Attenuation * SpotLights[i].Intensity;
    }

    // Directional
    CalculateDirectionalLight(DirectionalLights[0].Direction, Direction);

    // Transform tangent space
    Direction = mul(Direction, Input.TBN);
    
    ComputeLambert(DirectionalLights[0].Color, Direction, Normal, LightDiffuseColor);

    TotalDiffuse += LightDiffuseColor * DirectionalLights[0].Intensity;

    // Total Color
    TotalColor += MatDiffuseColor * TotalDiffuse;
    
#elif LIGHTING_MODEL_PHONG
    for (int i = 0; i < CulledLightNum; i++)
    {
        FPointLight PointLightFromBuffer = PointLightBufferList[CulledLightIndex[i]];
        CalculatePointLight(PointLightFromBuffer.Position, Input.worldPos, PointLightFromBuffer.AttenuationRadius, PointLightFromBuffer.Falloff, Direction, Attenuation);

        // Transform tangent space
        Direction = mul(Direction, Input.TBN);
        float3 ViewDirection = normalize(CameraPosition - Input.worldPos);
        ViewDirection = mul(ViewDirection, Input.TBN);

        ComputeBlinnPhong(PointLightFromBuffer.Color, Direction, ViewDirection, Normal, Material.SpecularScalar, LightDiffuseColor, LightSpecularColor);

        TotalDiffuse += LightDiffuseColor * Attenuation * PointLightFromBuffer.Intensity;
        TotalSpecular += LightSpecularColor * Attenuation * PointLightFromBuffer.Intensity;
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
        
        ComputeBlinnPhong(SpotLights[i].Color, Direction, ViewDirection, Normal, Material.SpecularScalar, LightDiffuseColor, LightSpecularColor);

        TotalDiffuse += LightDiffuseColor * Attenuation * SpotLights[i].Intensity;
        TotalSpecular += LightSpecularColor * Attenuation * SpotLights[i].Intensity;
    }

    // Directional
    CalculateDirectionalLight(DirectionalLights[0].Direction, Direction);

    // Transform tangent space
    Direction = mul(Direction, Input.TBN);
    float3 ViewDirection = normalize(CameraPosition - Input.worldPos);
    ViewDirection = mul(ViewDirection, Input.TBN);
    
    ComputeBlinnPhong(DirectionalLights[0].Color, Direction, ViewDirection, Normal, Material.SpecularScalar, LightDiffuseColor, LightSpecularColor);
    
    TotalDiffuse += LightDiffuseColor * DirectionalLights[0].Intensity;
    TotalSpecular += LightSpecularColor * DirectionalLights[0].Intensity;

    // Total Color
    TotalColor += MatDiffuseColor * TotalDiffuse + MatSpecularColor * TotalSpecular;
#endif

    Output.color = float4(TotalColor, 1);
    if (RenderFlag == 5)
    {
        Output.color = float4(TotalColor, 1);
        return Output;
    }
    
    // 선택
    if (isSelected)
    {
        Output.color += float4(0.02, 0.02, 0.02, 0);
    }
    
    return Output;
}
