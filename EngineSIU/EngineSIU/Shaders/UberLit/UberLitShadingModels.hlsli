// 물체 표면의 한 지점에 들어오는 빛이 어떻게 반사될것인지 계산

/**
 * Lambert Shader
 * 
 * Assign the result to DiffuseColor.
 * 
 * @param LightColor Color of light
 * @param LightDirection Direction of light
 * @param VertexNormal World vertex normal
 * @param DiffuseColor out DiffuseColor
 */
void ComputeLambert(float3 LightColor, float3 LightDirection, float3 VertexNormal, out float3 DiffuseColor)
{
    DiffuseColor = LightColor * max(dot(VertexNormal, (LightDirection / length(LightDirection))), 0.0f);
}

/**
 * Blinn Phong Shader
 * 
 * Assign the result to DiffuseColor & SpecularColor.
 * 
 * @param LightColor Color of light
 * @param LightDirection Direction of light
 * @param ViewDirection Vertex to Camera
 * @param VertexNormal World vertex normal
 * @param Shininess Specular shininess - material.specular.scalar
 * @param DiffuseColor out DiffuseColor
 * @param SpecularColor out SpecularColor
 */
void ComputeBlinnPhong(float3 LightColor, float3 LightDirection, float3 ViewDirection, float3 VertexNormal,
    float Shininess = 1.0f, out float3 DiffuseColor, out float3 SpecularColor)
{
    float Distance = length(LightDirection);
    
    float3 LightDirectionNormalized = LightDirection / Distance;
    float3 HalfVec = normalize(LightDirectionNormalized + ViewDirection);

    float NdotL = max(dot(VertexNormal, LightDirectionNormalized), 0.0f);
    float NdotH = max(dot(VertexNormal, HalfVec), 0.0f);
    
    DiffuseColor = LightColor * NdotL;
    SpecularColor = LightColor * pow(NdotH, Shininess);
}


/**
 * GouraudShader
 * - Use Lambert + Phong
 * 
 * @param VertexPosition World vertex position
 * @param VertexNormal World vertex normal
 * @param Shininess Color shininess scalar
 * @param AmbientLight FAmbientLight
 * @param DirectionalLight FDirectionalLight
 * @param PointLights List of pointlight
 * @param SpotLights List of spotlight
 * @param NumOfPointLight Number of pointlight
 * @param NumOfSpotLight Number of spotlight
 * @return Color3
 */
float3 ComputeGouraudShading(float3 VertexPosition, float3 VertexNormal, float Shininess,
                             FAmbientLight AmbientLight, FDirectionalLight DirectionalLight,
                             FPointLight PointLights[], FSpotLight SpotLights[],
                             int NumOfPointLight, int NumOfSpotLight)
{
    // Initialize by Ambient Light
    float3 ResultColor = float3(AmbientLight.Color) * AmbientLight.Intensity;
    float3 ViewDirection = normalize(CameraPosition - VertexPosition);
    
    /***********************
     * Directional Light   *
     ***********************/
    
    // Calculate direction vector.
    float3 LightDirection = normalize(-DirectionalLight.Direction);
    float3 DirectionalReflectDirection = reflect(-LightDirection, VertexNormal);

    // Calculate dot
    float DirectionalDiffuseDot = max(dot(VertexNormal, LightDirection), 0.0f);
    float DirectionalSpecularDot = pow(max(dot(DirectionalReflectDirection, ViewDirection), 0.0f), Shininess);

    // Sum color
    float3 DirectionalDiffuse = DirectionalLight.Color * DirectionalDiffuseDot;
    float3 DirectionalSpecular = DirectionalLight.Color * DirectionalSpecularDot;
    ResultColor += DirectionalLight.Intensity * (DirectionalDiffuse + DirectionalSpecular);
    
    /***********************
     * Point Light         *
     ***********************/
    for (int i = 0; i < NumOfPointLight; i++)
    {
        FPointLight Point = PointLights[i];

        // Calculate direction vector
        float3 ToPointLight = Point.Position - VertexPosition;
        float PointDistance = length(ToPointLight);
        float3 PointLightDirection = ToPointLight / PointDistance;
        float3 PointReflectDirection = reflect(PointLightDirection, VertexNormal);

        // Calculate attenuation
        float PointAttenuation = saturate(1.0f - PointDistance / Point.AttenuationRadius);
        PointAttenuation = pow(PointAttenuation, Point.Falloff);

        // Calculate dot
        float PointDiffuseDot = max(dot(VertexNormal, PointLightDirection), 0.0f);
        float PointSpecularDot = pow(max(dot(PointReflectDirection, ViewDirection), 0.0f), Shininess);

        // Sum color
        float3 PointDiffuse = Point.Color * PointDiffuseDot * PointAttenuation;
        float3 PointSpecular = Point.Color * PointSpecularDot * PointAttenuation;
        ResultColor += Point.Intensity * (PointDiffuse + PointSpecular);
    }
    
    /***********************
     * Spot Light          *
     ***********************/
    for (int i = 0; i < NumOfSpotLight; i++)
    {
        FSpotLight Spot = SpotLights[i];

        // Calculate direction vector
        float3 ToSpotLight = Spot.Position - VertexPosition;
        float SpotDistance = length(ToSpotLight);
        float3 SpotLightDirection = ToSpotLight / SpotDistance;
        float3 SpotReflectDirection = reflect(SpotLightDirection, VertexNormal);

        // Calculate radius attenuation
        float Attenuation = saturate(1.0 - SpotDistance / Spot.AttenuationRadius);
        Attenuation = pow(Attenuation, Spot.Falloff);

        // Calculate cone attenuation
        float CosAngle = dot(Spot.Direction, -LightDirection);
        float Outer = cos(radians(Spot.OuterConeAngle / 2.0f));
        float Inner = cos(radians(Spot.InnerConeAngle / 2.0f));

        float ConeAttenuation = saturate((CosAngle - Outer) / (Inner - Outer));
        ConeAttenuation = pow(ConeAttenuation, Spot.Falloff);
        
        float LightScale = Spot.Intensity * Attenuation * ConeAttenuation;

        // Calculate dot
        float SpotDiffuseDot = max(dot(VertexNormal, SpotLightDirection), 0.0f);
        float SpotSpecularDot = pow(max(dot(SpotReflectDirection, ViewDirection), 0.0f), Shininess);

        // Sum color
        float3 SpotDiffuse = Spot.Color * SpotDiffuseDot * LightScale;
        float3 SpotSpecular = Spot.Color * SpotSpecularDot;
        ResultColor += SpotDiffuse + SpotSpecular;
    }
    
    return ResultColor;
}
