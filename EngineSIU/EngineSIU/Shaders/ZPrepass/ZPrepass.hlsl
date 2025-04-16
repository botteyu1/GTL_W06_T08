// MatrixBuffer: 변환 행렬 관리
cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 ViewProjection;
};

struct VS_INPUT
{
    float3 position : POSITION; // 버텍스 위치
    float3 normal : NORMAL; // 버텍스 노멀
    float3 Tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR; // 버텍스 색상
    int materialIndex : MATERIAL_INDEX;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT mainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.position = mul(float4(input.position, 1.0), Model);
    output.position = mul(output.position, ViewProjection);
    return output;
}
