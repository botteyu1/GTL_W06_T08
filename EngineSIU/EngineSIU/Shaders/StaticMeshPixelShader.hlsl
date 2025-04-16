// staticMeshPixelShader.hlsl

Texture2D DiffuseTexture : register(t0);
Texture2D AmbientTexture : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D AlphaTexture : register(t3);
Texture2D EmissiveTexture : register(t4);
Texture2D RoughnessTexture : register(t5);
Texture2D NormalTexture : register(t6);

SamplerState Sampler : register(s0);

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float pad;
};

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    
    float3 AmbientColor;
    float DensityScalar;
    
    float3 SpecularColor;
    float SpecularScalar;
    
    float3 EmissiveColor;
    uint TextureFlag;

    float Sharpness;
    float Illum;

    float2 pad;
};

cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}
cbuffer FlagConstants : register(b4)
{
    int RenderFlag;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b5)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b6)
{
    float2 UVOffset;
    float2 TexturePad0;
}

#include "Light.hlsl"


struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치
    float4 color : COLOR; // 전달된 베이스 컬러
    float3 normal : NORMAL; // 월드 공간 노멀
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
    float3x3 TBN : TBN;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
};

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

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;
    output.UUID = UUID;

    float3 Normal = normalize(input.normal);
    float3 DiffuseColor = float3(1, 1, 1);
    float3 AmbientColor = float3(0, 0, 0);
    float3 SpecularColor = float3(0, 0, 0);
    float3 AlphaColor = float3(0, 0, 0);
    float3 EmissiveColor = float3(0, 0, 0);
    float3 RoughnessColor = float3(0, 0, 0);
    //float3 NormalColor = float3(0, 0, 0);


    float2 UV = input.texcoord + UVOffset; 

    // Diffuse
    if (Material.TextureFlag & (1 << 0))
    {
        DiffuseColor = DiffuseTexture.Sample(Sampler, UV).rgb;
    }

    // Ambient
    if (Material.TextureFlag & (1 << 1))
    {
        AmbientColor = AmbientTexture.Sample(Sampler, UV).rgb;
    }

    // Specular
    if (Material.TextureFlag & (1 << 2))
    {
        SpecularColor = SpecularTexture.Sample(Sampler, UV).rgb;
    }
    
    // Alpha
    if (Material.TextureFlag & (1 << 3))
    {
        AlphaColor = AlphaTexture.Sample(Sampler, UV).rgb;
    }

    // Emissive
    if (Material.TextureFlag & (1 << 4))
    {
        EmissiveColor = EmissiveTexture.Sample(Sampler, UV).rgb;
    }

    // Roughness
    if (Material.TextureFlag & (1 << 5))
    {
        RoughnessColor = RoughnessTexture.Sample(Sampler, UV).rgb;
    }
    
    // Normal (Bump)
    if (Material.TextureFlag & (1 << 6))
    {
        float3 SampledNormal = NormalTexture.Sample(Sampler, UV).rgb;
        SampledNormal = LinearToSRGB(SampledNormal);
        Normal = normalize(2.f * SampledNormal - 1.f);
        Normal = normalize(mul(mul(Normal, input.TBN), (float3x3) MInverseTranspose));
    }
    
    // 임시로 밖에 둠.
    DiffuseColor *= Material.DiffuseColor.rgb;
    
    float3 baseColor = DiffuseColor;

    if (RenderFlag == 0)
    {
        output.color = float4(baseColor, 1);
        return output;
    }
    // depth
    else if (RenderFlag == 2)
    {
        output.color = float4(1, 1, 1, 1);
        float z = input.position.z / input.position.w;
        output.color = float4(z,z,z, 1);
        return output;
    }
    // worldpos
    else if (RenderFlag == 3)
    {
        output.color = float4(input.worldPos/100 + 0.5 , 1);
        //output.color = float4(1, 1, 1, 1);
        return output;
    }
    // normal
    else if (RenderFlag == 4)
    {
        output.color = float4(Normal / 2 + 0.5, 1);
        return output;
    }
    // lit
    else
    {
        float3 lightRgb = Lighting(input.worldPos, Normal).rgb;
        float3 litColor = baseColor * lightRgb;
        output.color = float4(litColor, 1);
    }
    if (isSelected)
    {
        output.color += float4(0.02, 0.02, 0.02, 1);
    }
    return output;
}
