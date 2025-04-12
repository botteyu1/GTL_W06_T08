// UberLitTextures.hlsli
// Texture와 sampler를 정의하는 파일

Texture2D DiffuseTexture : register(t0);
Texture2D AmbientTexture : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D AlphaTexture : register(t3);
Texture2D EmissiveTexture : register(t4);
Texture2D RoughnessTexture : register(t5);
Texture2D NormalTexture : register(t6);

SamplerState Sampler : register(s0);
