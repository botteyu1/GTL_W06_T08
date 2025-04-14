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

#if LIGHTING_MODEL_LAMBERT
void ComputeLambert(float3 LightColor, float3 LightDirection, float3 VertexNormal, out float3 DiffuseColor)
{
    DiffuseColor = LightColor * max(dot(VertexNormal, (LightDirection / length(LightDirection))), 0.0f);
}
#endif

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
#if LIGHTING_MODEL_PHONG
void ComputeBlinnPhong(float3 LightColor, float3 LightDirection, float3 ViewDirection, float3 VertexNormal,
    float Shininess, out float3 OutDiffuseColor, out float3 OutSpecularColor)
{
    float Distance = length(LightDirection);
    
    float3 LightDirectionNormalized = LightDirection / Distance;
    float3 HalfVec = normalize(LightDirectionNormalized + ViewDirection);

    float NdotL = max(dot(VertexNormal, LightDirectionNormalized), 0.0f);
    float NdotH = max(dot(VertexNormal, HalfVec), 0.0f);
    
    OutDiffuseColor = LightColor * NdotL;
    OutSpecularColor = LightColor * pow(NdotH, Shininess);
}
#endif

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
                             FAmbientLight AmbientLight, FDirectionalLight DirectionalLights[NUM_MAX_DIRLIGHT],
                             FPointLight PointLights[NUM_MAX_POINTLIGHT], FSpotLight SpotLights[NUM_MAX_SPOTLIGHT],
                             int NumOfDirLight, int NumOfPointLight, int NumOfSpotLight)
{
    // Initialize by Ambient Light
    float3 ResultColor = float3(0, 0, 0);
    ResultColor += AmbientLight.Color * AmbientLight.Intensity;
    
    float3 ViewDirection = normalize(CameraPosition - VertexPosition);
    
    /***********************
     * Directional Light   *
     ***********************/

    if (NumOfDirLight > 0)
    {
        // Calculate direction vector.
        float3 LightDirection = normalize(-DirectionalLights[0].Direction);
        float3 DirectionalReflectDirection = reflect(-LightDirection, VertexNormal);

        // Calculate dot
        float DirectionalDiffuseDot = max(dot(VertexNormal, LightDirection), 0.0f);
        float DirectionalSpecularDot = pow(max(dot(DirectionalReflectDirection, ViewDirection), 0.0f), Shininess);

        // Sum color
        float3 DirectionalDiffuse = DirectionalLights[0].Color * DirectionalDiffuseDot;
        float3 DirectionalSpecular = DirectionalLights[0].Color * DirectionalSpecularDot;
        ResultColor += DirectionalLights[0].Intensity * (DirectionalDiffuse + DirectionalSpecular);
    }
    
    /***********************
     * Point Light         *
     ***********************/
    for (int i = 0; i < NumOfPointLight; i++)
    {
        FPointLight Point = PointLights[i];
        
        float3 Direction;
        float Attenuation;
        
        // Calculate direction vector
        CalculatePointLight(Point.Position, VertexPosition, Point.AttenuationRadius, Point.Falloff, Direction, Attenuation);
        float3 PointReflectDirection = reflect(Direction, VertexNormal);

        // Compare normal    light direction
        float NormalDotDirection = dot(VertexNormal, Direction);
        if (NormalDotDirection <= 0.0f)
        {
            continue;
        }
        
        
        // Calculate dot
        float PointDiffuseDot = max(dot(VertexNormal, Direction), 0.0f);
        float PointSpecularDot = pow(max(dot(PointReflectDirection, ViewDirection), 0.0f), Shininess);

        // Sum color
        float3 PointDiffuse = Point.Color * PointDiffuseDot * Attenuation;
        float3 PointSpecular = Point.Color * PointSpecularDot * Attenuation;
        ResultColor += Point.Intensity * (PointDiffuse + PointSpecular);
    }
    
    /***********************
     * Spot Light          *
     ***********************/
    for (int i = 0; i < NumOfSpotLight; i++)
    {
        FSpotLight Spot = SpotLights[i];

        float3 Direction;
        float Attenuation;

        // Calculate Spotlight direction
        CalculateSpotLight(Spot.Direction, Spot.Position, VertexPosition,
            Spot.AttenuationRadius, Spot.Falloff, Spot.InnerConeAngle, Spot.OuterConeAngle,
            Direction, Attenuation);
        float3 SpotReflectDirection = reflect(Direction, VertexNormal);

        float NormalDotDirection = dot(VertexNormal, Direction);
        if (NormalDotDirection <= 0.0f)
        {
            continue;
        }
        
        // Calculate dot
        float SpotDiffuseDot = max(dot(VertexNormal, Direction), 0.0f);
        float SpotSpecularDot = pow(max(dot(SpotReflectDirection, ViewDirection), 0.0f), Shininess);

        // Sum color
        float3 SpotDiffuse = Spot.Color * SpotDiffuseDot * Attenuation;
        float3 SpotSpecular = Spot.Color * SpotSpecularDot * Attenuation;
        ResultColor += Spot.Intensity * (SpotDiffuse + SpotSpecular);
    }
    
    return ResultColor;
}
