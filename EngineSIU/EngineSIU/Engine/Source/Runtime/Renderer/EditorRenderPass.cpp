#include "EditorRenderPass.h"

#include <d3dcompiler.h>
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/Classes/Actors/Player.h"
#include "Renderer.h"
#include "Engine/Classes/Components/Light/DirectionalLightComponent.h"
#include "Engine/Classes/Components/Light/PointLightComponent.h"
#include "Engine/Classes/Components/Light/SpotLightComponent.h"
#include "Engine/Classes/Components/ExponentialHeightFogComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/Engine.h"
#include "Engine/EditorEngine.h"
#include "World/World.h"
#include "Engine/Source/Runtime/Windows/D3D11RHI/DXDBufferManager.h"
#include "Engine/Source/Runtime/Windows/D3D11RHI/DXDShaderManager.h"

const float FEditorRenderPass::IconScale = 1.f;

void FEditorRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    if (InGraphics)
    {
        DeviceContext = InGraphics->DeviceContext;
    }
    ShaderManager = InShaderManager;
    CreateShaders();
    CreateBuffers();
    CreateConstantBuffers();
    SetGridParameter(10.f, 200);
}

void FEditorRenderPass::Release()
{
    ReleaseShaders();
}

void FEditorRenderPass::CreateShaders()
{
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    HRESULT hr;
    /////////////////////////////
    // 기즈모
    //D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gizmoVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    //if (errorBlob)
    //{
    //    OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    //    errorBlob->Release();
    //}
    //Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Gizmo.Vertex);
    //
    //D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gizmoPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    //Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Gizmo.Pixel);

    //D3D11_INPUT_ELEMENT_DESC layout[] = {
    //    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    //};

    //Graphics->Device->CreateInputLayout(
    //    layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &Resources.Shaders.Gizmo.Layout
    //);
    //Resources.Shaders.Gizmo.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    //VertexShaderCSO->Release();
    //PixelShaderCSO->Release();

    //VertexShaderCSO = nullptr;
    //PixelShaderCSO = nullptr;

    /////////////////////////////
    // axisline
    hr = ShaderManager->AddVertexShader(ShaderNameAxis, ShaderPath, "axisVS", defines);
    if (FAILED(hr))
    {
        return;
    }
    
    hr = ShaderManager->AddPixelShader(ShaderNameAxis, ShaderPath, "axisPS", defines);
    if (FAILED(hr))
    {
        return;
    }
    
    /////////////////////////////
    // AABB
    D3D11_INPUT_ELEMENT_DESC PositionOnlyLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    hr = ShaderManager->AddVertexShaderAndInputLayout(ShaderNameAABB, ShaderPath, "aabbVS", PositionOnlyLayout, ARRAYSIZE(PositionOnlyLayout), defines);
    if (FAILED(hr))
    {
        return;
    }

    hr = ShaderManager->AddPixelShader(ShaderNameAABB, ShaderPath, "aabbPS", defines);
    if (FAILED(hr))
    {
        return;
    }
    /////////////////////////////
    // Sphere
    hr = ShaderManager->AddVertexShaderAndInputLayout(ShaderNameSphere, ShaderPath, "sphereVS", PositionOnlyLayout, ARRAYSIZE(PositionOnlyLayout), defines);
    if (FAILED(hr))
    {
        return;
    }

    hr = ShaderManager->AddPixelShader(ShaderNameSphere, ShaderPath, "spherePS", defines);
    if (FAILED(hr))
    {
        return;
    }

    /////////////////////////////
    // Cone
    hr = ShaderManager->AddVertexShaderAndInputLayout(ShaderNameCone, ShaderPath, "coneVS", PositionOnlyLayout, ARRAYSIZE(PositionOnlyLayout), defines);
    if (FAILED(hr))
    {
        return;
    }

    hr = ShaderManager->AddPixelShader(ShaderNameCone, ShaderPath, "conePS", defines);
    if (FAILED(hr))
    {
        return;
    }

    ///////////////////////////////
    //// Grid
    hr = ShaderManager->AddVertexShader(ShaderNameGrid, ShaderPath, "gridVS", defines);
    if (FAILED(hr))
    {
        return;
    }

    hr = ShaderManager->AddPixelShader(ShaderNameGrid, ShaderPath, "gridPS", defines);
    if (FAILED(hr))
    {
        return;
    }
    /////////////////////////////
    // Icons
    hr = ShaderManager->AddVertexShader(ShaderNameIcon, ShaderPath, "iconVS", defines);
    if (FAILED(hr))
    {
        return;
    }

    hr = ShaderManager->AddPixelShader(ShaderNameIcon, ShaderPath, "iconPS", defines);
    if (FAILED(hr))
    {
        return;
    }


    /////////////////////////////
    // Arrow
    D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    hr = ShaderManager->AddVertexShaderAndInputLayout(ShaderNameArrow, ShaderPath, "arrowVS", StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), defines);
    if (FAILED(hr))
    {
        return;
    }

    hr = ShaderManager->AddPixelShader(ShaderNameArrow, ShaderPath, "arrowPS", defines);
    if (FAILED(hr))
    {
        return;
    }




    //D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "arrowVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    //if (errorBlob)
    //{
    //    OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    //    errorBlob->Release();
    //}
    //Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Arrow.Vertex);

    //D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "arrowPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);
    //if (errorBlob)
    //{
    //    OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    //    errorBlob->Release();
    //}
    //Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Arrow.Pixel);

    //// gizmo의 layout을 이용
    //Resources.Shaders.Arrow.Layout = Resources.Shaders.Gizmo.Layout;

    //Resources.Shaders.Arrow.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    //VertexShaderCSO->Release();
    //PixelShaderCSO->Release();

    //VertexShaderCSO = nullptr;
    //PixelShaderCSO = nullptr;
}
//
//void FEditorRenderPass::PrepareShader(FShaderResource ShaderResource) const
//{
//
//}

void FEditorRenderPass::ReleaseShaders()
{


}

void FEditorRenderPass::CreateBuffers()
{
    ////////////////////////////////////
    // Box 버퍼 생성
    TArray<FVector> CubeFrameVertices;
    CubeFrameVertices.Add({ -1.f, -1.f, -1.f}); // 0
    CubeFrameVertices.Add({ -1.f, 1.f, -1.f}); // 1
    CubeFrameVertices.Add({ 1.f, -1.f, -1.f}); // 2
    CubeFrameVertices.Add({ 1.f, 1.f, -1.f}); // 3
    CubeFrameVertices.Add({ -1.f, -1.f, 1.f}); // 4
    CubeFrameVertices.Add({ 1.f, -1.f, 1.f}); // 5
    CubeFrameVertices.Add({ -1.f, 1.f, 1.f}); // 6
    CubeFrameVertices.Add({ 1.f, 1.f, 1.f}); // 7

    TArray<uint32> CubeFrameIndices = {
        // Bottom face
        0, 1, 1, 3, 3, 2, 2, 0,
        // Top face
        4, 6, 6, 7, 7, 5, 5, 4,
        // Side faces
        0, 4, 1, 6, 2, 5, 3, 7
    };

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(FVector) * CubeFrameVertices.Num();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = CubeFrameVertices.GetData();

    HRESULT hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(uint32) * CubeFrameIndices.Num();

    initData.pSysMem = CubeFrameIndices.GetData();

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Index);

    Resources.Primitives.Box.NumVertices = CubeFrameVertices.Num();
    Resources.Primitives.Box.VertexStride = sizeof(FVector);
    Resources.Primitives.Box.NumIndices = CubeFrameIndices.Num();

    ////////////////////////////////////
    // Sphere 버퍼 생성
    FVector SphereFrameVertices[] =
    {
        {1.0, 0.0, 0},
        {0.9795299412524945, 0.20129852008866006, 0},
        {0.9189578116202306, 0.39435585511331855, 0},
        {0.8207634412072763, 0.5712682150947923, 0},
        {0.6889669190756866, 0.72479278722912, 0},
        {0.5289640103269624, 0.8486442574947509, 0},
        {0.3473052528448203, 0.9377521321470804, 0},
        {0.1514277775045767, 0.9884683243281114, 0},
        {-0.05064916883871264, 0.9987165071710528, 0},
        {-0.2506525322587204, 0.9680771188662043, 0},
        {-0.4403941515576344, 0.8978045395707416, 0},
        {-0.6121059825476629, 0.7907757369376985, 0},
        {-0.758758122692791, 0.6513724827222223, 0},
        {-0.8743466161445821, 0.48530196253108104, 0},
        {-0.9541392564000488, 0.29936312297335804, 0},
        {-0.9948693233918952, 0.10116832198743228, 0},
        {-0.9948693233918952, -0.10116832198743204, 0},
        {-0.9541392564000489, -0.29936312297335776, 0},
        {-0.8743466161445822, -0.4853019625310808, 0},
        {-0.7587581226927911, -0.651372482722222, 0},
        {-0.6121059825476627, -0.7907757369376986, 0},
        {-0.44039415155763423, -0.8978045395707417, 0},
        {-0.2506525322587205, -0.9680771188662043, 0},
        {-0.05064916883871266, -0.9987165071710528, 0},
        {0.15142777750457667, -0.9884683243281114, 0},
        {0.3473052528448203, -0.9377521321470804, 0},
        {0.5289640103269624, -0.8486442574947509, 0},
        {0.6889669190756865, -0.72479278722912, 0},
        {0.8207634412072763, -0.5712682150947924, 0},
        {0.9189578116202306, -0.3943558551133187, 0},
        {0.9795299412524945, -0.20129852008866028, 0},
        {1, 0, 0},
        {1.0, 0, 0.0},
        {0.9795299412524945, 0, 0.20129852008866006},
        {0.9189578116202306, 0, 0.39435585511331855},
        {0.8207634412072763, 0, 0.5712682150947923},
        {0.6889669190756866, 0, 0.72479278722912},
        {0.5289640103269624, 0, 0.8486442574947509},
        {0.3473052528448203, 0, 0.9377521321470804},
        {0.1514277775045767, 0, 0.9884683243281114},
        {-0.05064916883871264, 0, 0.9987165071710528},
        {-0.2506525322587204, 0, 0.9680771188662043},
        {-0.4403941515576344, 0, 0.8978045395707416},
        {-0.6121059825476629, 0, 0.7907757369376985},
        {-0.758758122692791, 0, 0.6513724827222223},
        {-0.8743466161445821, 0, 0.48530196253108104},
        {-0.9541392564000488, 0, 0.29936312297335804},
        {-0.9948693233918952, 0, 0.10116832198743228},
        {-0.9948693233918952, 0, -0.10116832198743204},
        {-0.9541392564000489, 0, -0.29936312297335776},
        {-0.8743466161445822, 0, -0.4853019625310808},
        {-0.7587581226927911, 0, -0.651372482722222},
        {-0.6121059825476627, 0, -0.7907757369376986},
        {-0.44039415155763423, 0, -0.8978045395707417},
        {-0.2506525322587205, 0, -0.9680771188662043},
        {-0.05064916883871266, 0, -0.9987165071710528},
        {0.15142777750457667, 0, -0.9884683243281114},
        {0.3473052528448203, 0, -0.9377521321470804},
        {0.5289640103269624, 0, -0.8486442574947509},
        {0.6889669190756865, 0, -0.72479278722912},
        {0.8207634412072763, 0, -0.5712682150947924},
        {0.9189578116202306, 0, -0.3943558551133187},
        {0.9795299412524945, 0, -0.20129852008866028},
        {1, 0, 0},
        {0, 1.0, 0.0},
        {0, 0.9795299412524945, 0.20129852008866006},
        {0, 0.9189578116202306, 0.39435585511331855},
        {0, 0.8207634412072763, 0.5712682150947923},
        {0, 0.6889669190756866, 0.72479278722912},
        {0, 0.5289640103269624, 0.8486442574947509},
        {0, 0.3473052528448203, 0.9377521321470804},
        {0, 0.1514277775045767, 0.9884683243281114},
        {0, -0.05064916883871264, 0.9987165071710528},
        {0, -0.2506525322587204, 0.9680771188662043},
        {0, -0.4403941515576344, 0.8978045395707416},
        {0, -0.6121059825476629, 0.7907757369376985},
        {0, -0.758758122692791, 0.6513724827222223},
        {0, -0.8743466161445821, 0.48530196253108104},
        {0, -0.9541392564000488, 0.29936312297335804},
        {0, -0.9948693233918952, 0.10116832198743228},
        {0, -0.9948693233918952, -0.10116832198743204},
        {0, -0.9541392564000489, -0.29936312297335776},
        {0, -0.8743466161445822, -0.4853019625310808},
        {0, -0.7587581226927911, -0.651372482722222},
        {0, -0.6121059825476627, -0.7907757369376986},
        {0, -0.44039415155763423, -0.8978045395707417},
        {0, -0.2506525322587205, -0.9680771188662043},
        {0, -0.05064916883871266, -0.9987165071710528},
        {0, 0.15142777750457667, -0.9884683243281114},
        {0, 0.3473052528448203, -0.9377521321470804},
        {0, 0.5289640103269624, -0.8486442574947509},
        {0, 0.6889669190756865, -0.72479278722912},
        {0, 0.8207634412072763, -0.5712682150947924},
        {0, 0.9189578116202306, -0.3943558551133187},
        {0, 0.9795299412524945, -0.20129852008866028},
        {0, 1, 0}
    };

    uint32 SphereFrameIndices[] =
    {
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
        11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20,
        21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30,
        31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40,
        41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50,
        51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60,
        61, 61, 62, 62, 63, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70,
        71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80,
        81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90,
        91, 91, 92, 92, 93, 93, 94, 94, 95
    };

    // 버텍스 버퍼 생성
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(SphereFrameVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    initData = {};
    initData.pSysMem = SphereFrameVertices;

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Sphere.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(SphereFrameIndices);

    initData.pSysMem = SphereFrameIndices;

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Sphere.Index);

    Resources.Primitives.Sphere.NumVertices = ARRAYSIZE(SphereFrameVertices);
    Resources.Primitives.Sphere.VertexStride = sizeof(FVector);
    Resources.Primitives.Sphere.NumIndices = ARRAYSIZE(SphereFrameIndices);



    ////////////////////////////////////
    // Cone 버퍼 생성
    // 0,0,0이 Apex
    // z=1이고, xy에서 r=1인 원이 밑변
    TArray<FVector> ConeVertices;
    ConeVertices.Add({0.0f, 0.0f, 0.0f}); // Apex
    for (int i = 0; i < NumSegments; i++)
    {
        // hlsl 내부에서 계산
        //float angle = 2.0f * 3.1415926535897932f * i / (float)NuzmSegments;
        //float x = cos(angle);
        //float y = sin(angle);
        float x = 0;
        float y = 0;
        ConeVertices.Add({x, y, 1.0f}); // Bottom
    }
    TArray<uint32> ConeIndices;
    uint32 vertexOffset0 = 1;
    // apex -> 밑면으로 가는 line
    for (int i = 0; i < NumSegments; i++)
    {
        ConeIndices.Add(0);
        ConeIndices.Add(vertexOffset0 + i);
    }
    // 밑변
    for (int i = 0; i < NumSegments; i++)
    {
        ConeIndices.Add(vertexOffset0 + i);
        ConeIndices.Add(vertexOffset0 + (i + 1) % NumSegments);
    }

    // cone을 덮는 sphere
    // xz plane
    //float deltaAngle = 2.0f * 3.1415926535897932f / (float)NumSegments;
    //float offsetAngle = deltaAngle * NumSegments / 8; // 45도 부터 시작
    for (int i = 0; i < NumSegments + 1; i++)
    {
        //float angle = 2.0f * 3.1415926535897932f * i / (float)NumSegments + offsetAngle;
        //float x = cos(angle) * sqrt(2.f);
        //float z = sin(angle) * sqrt(2.f);
        float x = 0;
        float z = 0;
        ConeVertices.Add({ x, 0, z });
    }
    uint32 vertexOffset1 = NumSegments + vertexOffset0;
    for (int i = 0; i < NumSegments; i++)
    {
        ConeIndices.Add(vertexOffset1 + i);
        ConeIndices.Add(vertexOffset1 + (i + 1));
    }
    // yz plane
    for (int i = 0; i < NumSegments + 1; i++)
    {
        //float angle = 2.0f * 3.1415926535897932f * i / (float)NumSegments + offsetAngle;
        //float y = cos(angle) * sqrt(2.f);
        //float z = sin(angle) * sqrt(2.f);
        float y = 0;
        float z = 0;
        ConeVertices.Add({ 0, y, z });
    }
    uint32 vertexOffset2 = NumSegments + 1 + vertexOffset1;
    for (int i = 0; i < NumSegments; i++)
    {
        ConeIndices.Add(vertexOffset2 + i);
        ConeIndices.Add(vertexOffset2 + (i + 1));
    }
    
    // 버텍스 버퍼 생성
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = ConeVertices.Num() * sizeof(FVector);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    initData = {};
    initData.pSysMem = ConeVertices.GetData();

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Cone.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = ConeIndices.Num() * sizeof(UINT);

    initData.pSysMem = ConeIndices.GetData();

    hr = Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Cone.Index);

    Resources.Primitives.Cone.NumVertices = ConeVertices.Num();
    Resources.Primitives.Cone.VertexStride = sizeof(FVector);
    Resources.Primitives.Cone.NumIndices = ConeIndices.Num();

}

void FEditorRenderPass::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferCamera);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Camera00);

    // 16개 고정
    // 그려야할 대상이 더 많을 경우 16개씩 쪼개서 사용
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugAABB) * ConstantBufferSizeAABB;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.AABB13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugSphere) * ConstantBufferSizeSphere;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Sphere13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugCone) * ConstantBufferSizeCone;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Cone13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugGrid);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Grid13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugIcon) * ConstantBufferSizeIcon;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Icon13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugArrow);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Arrow13);

}

void FEditorRenderPass::PrepareRendertarget()
{
    // RenderTarget 그대로 이용
    //DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
}

void FEditorRenderPass::PrepareComponents()
{
    Resources.Components.StaticMesh.Empty();
    Resources.Components.DirLight.Empty();
    Resources.Components.PointLight.Empty();
    Resources.Components.SpotLight.Empty();
    Resources.Components.Fog.Empty();
    //Resources.Components.DirLight = nullptr;
    //Resources.Components.PointLight = nullptr;
    //Resources.Components.SpotLight = nullptr;
    //Resources.Components.Fog = nullptr;
    // gizmo 제외하고 넣기

    if (GEngine->ActiveWorld->WorldType != EWorldType::Editor)
    {
        return;
    }
    
    UEditorEngine* EditorEngine = Cast< UEditorEngine>(GEngine);
    if (!EditorEngine)
    {
        return;
    }

    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.StaticMesh.Add(iter);
        }
    }

    for (const auto iter : TObjectRange<UDirectionalLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.DirLight.Add(iter);
        }
    }

    for (const auto iter : TObjectRange<UPointLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.PointLight.Add(iter);
        }
    }


    for (const auto iter : TObjectRange<USpotLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.SpotLight.Add(iter);
        }
    }

    for (const auto iter : TObjectRange<UExponentialHeightFogComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
           Resources.Components.Fog.Add(iter);
        }
    }


    //if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
    //{
    //    if (USceneComponent* SelectedComponent = EditorEngine->GetSelectedComponent())
    //    {
    //        if (SelectedComponent->IsA<UDirectionalLightComponent>())
    //        {
    //            Resources.Components.DirLight = Cast<UDirectionalLightComponent>(SelectedComponent);
    //        }
    //        else if (SelectedComponent->IsA<UPointLightComponent>())
    //        {
    //            Resources.Components.PointLight = Cast<UPointLightComponent>(SelectedComponent);
    //        }
    //        else if (SelectedComponent->IsA<USpotLightComponent>())
    //        {
    //            Resources.Components.SpotLight = Cast<USpotLightComponent>(SelectedComponent);
    //        }
    //        else if (SelectedComponent->IsA<UExponentialHeightFogComponent>())
    //        {
    //            Resources.Components.Fog = Cast<UExponentialHeightFogComponent>(SelectedComponent);
    //        }
    //    }
    //}
}

void FEditorRenderPass::PrepareConstantbufferGlobal()
{
    if (Resources.ConstantBuffers.Camera00)
    {
        DeviceContext->VSSetConstantBuffers(0, 1, &Resources.ConstantBuffers.Camera00);
        DeviceContext->PSSetConstantBuffers(0, 1, &Resources.ConstantBuffers.Camera00);
    }
}

void FEditorRenderPass::UpdateConstantbufferGlobal(FConstantBufferCamera Buffer)
{
    if (Resources.ConstantBuffers.Camera00)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Camera00, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        DeviceContext->Unmap(Resources.ConstantBuffers.Camera00, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::Render(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    static bool isLoaded = false;
    if (!isLoaded)
    {
        LazyLoad();
        isLoaded = true;
    }
    UpdateShaders();

    PrepareRendertarget();
    PrepareComponents();
    PrepareConstantbufferGlobal();
    DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());

    FConstantBufferCamera buf;
    buf.ViewMatrix = ActiveViewport->GetViewMatrix();
    buf.ProjMatrix = ActiveViewport->GetProjectionMatrix();
    buf.CameraPos = ActiveViewport->ViewTransformPerspective.GetLocation();
    buf.CameraLookAt = ActiveViewport->ViewTransformPerspective.GetLookAt();
    UpdateConstantbufferGlobal(buf);

    ID3D11DepthStencilState* DepthStateEnable = Graphics->DepthStencilStateTestWriteEnable;
    DeviceContext->OMSetDepthStencilState(DepthStateEnable, 0);

    RenderAABBInstanced(ActiveViewport);
    RenderPointlightInstanced(ActiveViewport);
    RenderSpotlightInstanced(ActiveViewport);
    RenderAxis();
    RenderGrid(ActiveViewport);

    // 기즈모는 depth 무시
    ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStencilStateTestWriteDisable;
    DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    RenderIcons(ActiveViewport);
    RenderArrows();
}

void FEditorRenderPass::SetGridParameter(float Spacing, uint32 GridCount)
{
    CurrentGridSettings.GridSpacing = Spacing;
    CurrentGridSettings.GridCount = GridCount;
    CurrentGridSettings.GridOrigin = { 0,0,0 };
}

//void FEditorRenderPass::RenderGizmos(const UWorld* World)
//{
//    PrepareShader(Resources.Shaders.Gizmo);
//
//    //Renderer->PrepareShader(Renderer->RenderResources.Shaders.StaticMesh);
//    //Renderer->PrepareShader(Resources.Shaders.Gizmo);
//
//    if (!World->GetSelectedActor())
//    {
//        return;
//    }
//
//    TArray<UGizmoBaseComponent*> GizmoObjs;
//
//    for (const auto iter : TObjectRange<UGizmoBaseComponent>())
//    {
//        GizmoObjs.Add(iter);
//    }
//
//    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
//    DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);
//    for (UGizmoBaseComponent* GizmoComp : GizmoObjs)
//    {
//        if (AActor* PickedActor = World->GetSelectedActor())
//        {
//            std::shared_ptr<FEditorViewportClient> activeViewport = GEngine->GetLevelEditor()->GetActiveViewportClient();
//            if (activeViewport->IsPerspective())
//            {
//                float scalar = abs(
//                    (activeViewport->ViewTransformPerspective.GetLocation() - PickedActor->GetRootComponent()->GetLocalLocation()).Magnitude()
//                );
//                scalar *= 0.1f;
//                GizmoComp->SetRelativeScale(FVector(scalar, scalar, scalar));
//            }
//            else
//            {
//                float scalar = activeViewport->orthoSize * 0.1f;
//                GizmoComp->SetRelativeScale(FVector(scalar, scalar, scalar));
//            }
//        }
//        if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowX ||
//            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowY ||
//            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
//            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
//            continue;
//        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleX ||
//            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleY ||
//            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
//            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
//            continue;
//        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleX ||
//            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleY ||
//            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleZ)
//            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
//            continue;
//        FMatrix Model = GizmoComp->GetComponentTransform();
//        {
//            FConstantBufferActor buf;
//            buf.IsSelectedActor = 0;
//            buf.UUID = GizmoComp->EncodeUUID() / 255.0f;
//            Renderer->UpdateConstantbufferActor(buf);
//        }
//        {
//            FConstantBufferMesh buf;
//            buf.ModelMatrix = Model;
//            if (GizmoComp == World->GetPickingGizmo())
//            {
//                buf.IsSelectedMesh = 1;
//            }
//            else
//            {
//                buf.IsSelectedMesh = 0;
//            }
//            Renderer->UpdateConstantbufferMesh(buf);
//        }
//        // TODO: 기즈모 선택효과 안보임
//        // 현재 RenderPrimitive()에서 다시 SelectedMesh를 갱신하는데, 안에서 기즈모를 인식시킬 수 없음.
//
//        if (!GizmoComp->GetStaticMesh()) continue;
//
//        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
//        if (renderData == nullptr) continue;
//
//        Renderer->RenderPrimitive(Model, renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials(), -1);
//
//    }
//
//    DeviceContext->RSSetState(GetCurrentRasterizer());
//
//#pragma region GizmoDepth
//    ID3D11DepthStencilState* originalDepthState = DepthStencilStateTestWriteEnable;
//    DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
//#pragma endregion GizmoDepth
//}
//
//void FEditorRenderPass::PrepareShaderGizmo()
//{
//    
//}
//
//void FEditorRenderPass::PrepareConstantbufferGizmo()
//{
//}

void FEditorRenderPass::RenderAxis()
{
    ShaderManager->SetVertexShader(ShaderNameAxis, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameAxis, DeviceContext);
    DeviceContext->IASetInputLayout(nullptr);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    DeviceContext->Draw(6, 0);
}

void FEditorRenderPass::RenderAABBInstanced(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (!(ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))) return;

    ShaderManager->SetVertexShaderAndInputLayout(ShaderNameAABB, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameAABB, DeviceContext);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Box.Vertex, &Resources.Primitives.Box.VertexStride, &offset);
    DeviceContext->IASetIndexBuffer(Resources.Primitives.Box.Index, DXGI_FORMAT_R32_UINT, 0);

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugAABB> BufferAll;
    for (UStaticMeshComponent* SMComp : Resources.Components.StaticMesh)
    {
        // 현재 bounding box를 갱신안해주고있음 : 여기서 직접 갱신
        SMComp->UpdateAABB();
        FConstantBufferDebugAABB b;
        b.Position = SMComp->GetWorldBoundingBox().GetPosition();
        b.Extent = SMComp->GetWorldBoundingBox().GetExtent();
        BufferAll.Add(b);
    }

    PrepareConstantbufferAABB();
    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeAABB) * ConstantBufferSizeAABB; ++i)
    {
        TArray<FConstantBufferDebugAABB> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferAABBInstanced(SubBuffer);
            DeviceContext->DrawIndexedInstanced(Resources.Primitives.Box.NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}

void FEditorRenderPass::PrepareConstantbufferAABB()
{
    if (Resources.ConstantBuffers.AABB13)
    {
        DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.AABB13);
    }
}

void FEditorRenderPass::UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer)
{
    if (Buffer.Num() > ConstantBufferSizeAABB)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.AABB13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.AABB13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffer.GetData(), sizeof(FConstantBufferDebugAABB) * Buffer.Num()); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.AABB13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::RenderPointlightInstanced(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (!(ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_LightVisualize))) return;

    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
    if (!EditorEngine)
    {
        return;
    }

    ShaderManager->SetVertexShaderAndInputLayout(ShaderNameSphere, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameSphere, DeviceContext);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Sphere.Vertex, &Resources.Primitives.Sphere.VertexStride, &offset);
    DeviceContext->IASetIndexBuffer(Resources.Primitives.Sphere.Index, DXGI_FORMAT_R32_UINT, 0);

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugSphere> BufferAll;
    for (UPointLightComponent* PointLightComp : Resources.Components.PointLight)
    {
        //if (PointLightComp == EditorEngine->GetSelectedComponent())
        //{
            FConstantBufferDebugSphere b;
            b.Position = PointLightComp->GetWorldLocation();
            b.Radius = PointLightComp->GetAttenuationRadius();
            b.Color = FLinearColor(149.f / 255.f, 198.f / 255.f, 255.f / 255.f, 255.f / 255.f);

            BufferAll.Add(b);
            //break;
        //}
    }

    PrepareConstantbufferPointlight();
    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeSphere) * ConstantBufferSizeSphere; ++i)
    {
        TArray<FConstantBufferDebugSphere> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeSphere; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferPointlightInstanced(SubBuffer);
            DeviceContext->DrawIndexedInstanced(Resources.Primitives.Sphere.NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}

void FEditorRenderPass::PrepareConstantbufferPointlight()
{
    if (Resources.ConstantBuffers.Sphere13)
    {
        DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Sphere13);
    }
}

void FEditorRenderPass::UdpateConstantbufferPointlightInstanced(TArray<FConstantBufferDebugSphere> Buffer)
{
    if (Buffer.Num() > ConstantBufferSizeSphere)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.Sphere13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Sphere13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffer.GetData(), sizeof(FConstantBufferDebugSphere) * Buffer.Num()); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.Sphere13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::RenderSpotlightInstanced(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (!(ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_LightVisualize))) return;

    // TODO : 현재 z값이 과도하게 크면 cone의 둥근 부분이 구형이 아님
    // 따라서 따로 그려줘서 곡률이 일정하게 만들어야 할거같음
    // 아니고 그냥 셰이더에서 할수있을거같기도 함...
    ShaderManager->SetVertexShaderAndInputLayout(ShaderNameCone, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameCone, DeviceContext);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Cone.Vertex, &Resources.Primitives.Cone.VertexStride, &offset);
    DeviceContext->IASetIndexBuffer(Resources.Primitives.Cone.Index, DXGI_FORMAT_R32_UINT, 0);

    constexpr FLinearColor OuterColor = FLinearColor(149.f/255.f, 198.f/255.f, 255.f/255.f, 255.f/255.f);
    constexpr FLinearColor InnerColor = FLinearColor(0.777f, 1.0f, 1.0f, 1.0f);
    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugCone> BufferAll;
    for (USpotLightComponent* SpotComp : Resources.Components.SpotLight)
    {
        if (SpotComp == Cast<UEditorEngine>(GEngine)->GetSelectedComponent())
        {
            FConstantBufferDebugCone b;
            b.ApexPosiiton = SpotComp->GetWorldLocation();
            b.Radius = SpotComp->GetAttenuationRadius();
            b.Direction = SpotComp->GetForwardVector();
            b.Angle = FMath::DegreesToRadians(SpotComp->GetInnerConeAngle());
            b.Color = InnerColor;
            BufferAll.Add(b);

            b.Angle = FMath::DegreesToRadians(SpotComp->GetOuterConeAngle());
            b.Color = OuterColor;
            BufferAll.Add(b);
        }
    }

    PrepareConstantbufferSpotlight();
    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeCone) * ConstantBufferSizeCone; ++i)
    {
        TArray<FConstantBufferDebugCone> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferSpotlightInstanced(SubBuffer);
            DeviceContext->DrawIndexedInstanced(Resources.Primitives.Cone.NumIndices, 2, 0, 0, 0);
        }
    }
}

void FEditorRenderPass::PrepareConstantbufferSpotlight()
{
    if (Resources.ConstantBuffers.Cone13)
    {
        DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Cone13);
    }
}

void FEditorRenderPass::UdpateConstantbufferSpotlightInstanced(TArray<FConstantBufferDebugCone> Buffer)
{
    if (Buffer.Num() > ConstantBufferSizeCone)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.Cone13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Cone13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffer.GetData(), sizeof(FConstantBufferDebugCone) * Buffer.Num()); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.Cone13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::RenderGrid(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    ShaderManager->SetVertexShader(ShaderNameGrid, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameGrid, DeviceContext);
    PrepareConstantbufferGlobal();
    PrepareConstantbufferGrid();
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    FMatrix view = ActiveViewport->GetViewMatrix();
    FMatrix proj = ActiveViewport->GetProjectionMatrix();

    FConstantBufferCamera cameraBuf;
    cameraBuf.ViewMatrix = view;
    cameraBuf.ProjMatrix = proj;
    cameraBuf.CameraPos = ActiveViewport->ViewTransformPerspective.GetLocation();
    cameraBuf.CameraLookAt = {
        ActiveViewport->GetD3DViewport().Width,
        ActiveViewport->GetD3DViewport().Height,
        static_cast<float>(ActiveViewport->GetViewportType())
    };
    UpdateConstantbufferGlobal(cameraBuf);
    UdpateConstantbufferGrid(CurrentGridSettings);
    DeviceContext->DrawInstanced(2, CurrentGridSettings.GridCount, 0,0); // 내부에서 버텍스 사용중
}

void FEditorRenderPass::PrepareConstantbufferGrid()
{
    if (Resources.ConstantBuffers.Grid13)
    {
        DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Grid13);
    }
}

void FEditorRenderPass::UdpateConstantbufferGrid(FConstantBufferDebugGrid Buffer)
{
    if (Resources.ConstantBuffers.Grid13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Grid13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(FConstantBufferDebugGrid)); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.Grid13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

// 꼼수로 이미 로드된 리소스를 사용
// GUObjectArray에 안올라가게 우회
void FEditorRenderPass::LazyLoad()
{
// Resourcemanager에서 로드된 texture의 포인터를 가져옴
// FResourceMgr::Initialize에 추가되어야함
    Resources.IconTextures[IconType::DirectionalLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Icons/DirectionalLight_64x.png");
    Resources.IconTextures[IconType::PointLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Icons/PointLight_64x.png");
    Resources.IconTextures[IconType::SpotLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Icons/SpotLight_64x.png");
    Resources.IconTextures[IconType::ExponentialFog] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Icons/ExponentialHeightFog_64.png");
    Resources.IconTextures[IconType::AtmosphericFog] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Icons/AtmosphericFog_64.png");

//// Gizmo arrow 로드
    UStaticMesh* Mesh = FManagerOBJ::GetStaticMesh(L"Assets/GizmoTranslationZ.obj");
    Resources.Primitives.Arrow.Vertex = Mesh->GetRenderData()->VertexBuffer;
    Resources.Primitives.Arrow.Index = Mesh->GetRenderData()->IndexBuffer;
    Resources.Primitives.Arrow.NumVertices = Mesh->GetRenderData()->Vertices.Num();
    Resources.Primitives.Arrow.NumIndices = Mesh->GetRenderData()->Indices.Num();
    Resources.Primitives.Arrow.VertexStride = sizeof(Mesh->GetRenderData()->Vertices[0]);

}

void FEditorRenderPass::RenderIcons(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (!(ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Icons))) return;

    // ULightComponentBase::CheckRayIntersection에서도 수정 필요
    ShaderManager->SetVertexShader(ShaderNameIcon, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameIcon, DeviceContext);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    UINT offset = 0;
    // input vertex index 없음

    PrepareConstantbufferIcon();

    AActor* PickedActor = nullptr;
    if (UEditorEngine* Engine = Cast<UEditorEngine>(GEngine))
    {
        if (AEditorPlayer* player = Engine->GetEditorPlayer())
        {
            PickedActor = Engine->GetSelectedActor();
        }
    }

    for (UDirectionalLightComponent* DirLightComp : Resources.Components.DirLight)
    {
        FConstantBufferDebugIcon b;
        b.Position = DirLightComp->GetWorldLocation();
        b.Scale = IconScale;
        if (DirLightComp->GetOwner() == PickedActor)
        {
            b.Color = FLinearColor(0.5, 0.5, 0, 1);
        }
        else
        {
            b.Color = DirLightComp->GetLightColor();
        }
        UdpateConstantbufferIcon(b);
        UpdateTextureIcon(IconType::DirectionalLight);
        DeviceContext->Draw(6, 0); // 내부에서 버텍스 사용중
    }
    
    TArray<FConstantBufferDebugIcon> PointLightBuffers;
    for (UPointLightComponent* PointLightComp : Resources.Components.PointLight)
    {
        FConstantBufferDebugIcon b;
        b.Position = PointLightComp->GetWorldLocation();
        b.Scale = IconScale;
        if (PointLightComp->GetOwner() == PickedActor)
        {
            b.Color = FLinearColor(0.5, 0.5, 0, 1);
        }
        else
        {
            b.Color = PointLightComp->GetLightColor();
        }
        PointLightBuffers.Add(b);
    }

    UpdateTextureIcon(IconType::PointLight);
    int BufferIndex = 0;
    for (int i = 0; i < (1 + PointLightBuffers.Num() / ConstantBufferSizeIcon) * ConstantBufferSizeIcon; ++i)
    {
        TArray<FConstantBufferDebugIcon> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeIcon; ++j)
        {
            if (BufferIndex < PointLightBuffers.Num())
            {
                SubBuffer.Add(PointLightBuffers[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferIconInstanced(SubBuffer);
            DeviceContext->DrawInstanced(6, SubBuffer.Num(), 0, 0);
        }
    }
    
    for (USpotLightComponent* SpotLightComp : Resources.Components.SpotLight)
    {
        FConstantBufferDebugIcon b;
        b.Position = SpotLightComp->GetWorldLocation();
        b.Scale = IconScale;
        if (SpotLightComp->GetOwner() == PickedActor)
        {
            b.Color = FLinearColor(0.5, 0.5, 0, 1);
        }
        else
        {
            b.Color = SpotLightComp->GetLightColor();
        }
        UdpateConstantbufferIcon(b);
        UpdateTextureIcon(IconType::SpotLight);
        DeviceContext->Draw(6, 0); // 내부에서 버텍스 사용중
    }

    for (UExponentialHeightFogComponent* FogComp : Resources.Components.Fog)
    {
        FConstantBufferDebugIcon b;
        b.Position = FogComp->GetWorldLocation();
        b.Scale = IconScale;
        b.Color = FogComp->GetFogColor();

        UdpateConstantbufferIcon(b);
        UpdateTextureIcon(IconType::ExponentialFog);

        DeviceContext->Draw(6, 0); // 내부에서 버텍스 사용중
    }
}

void FEditorRenderPass::PrepareConstantbufferIcon()
{
    if (Resources.ConstantBuffers.Icon13)
    {
        DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Icon13);
        DeviceContext->PSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Icon13);
    }
}

void FEditorRenderPass::UdpateConstantbufferIcon(FConstantBufferDebugIcon Buffer)
{
    if (Resources.ConstantBuffers.Icon13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Icon13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(FConstantBufferDebugIcon)); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.Icon13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::UdpateConstantbufferIconInstanced(TArray<FConstantBufferDebugIcon> Buffers)
{
    if (Buffers.Num() > ConstantBufferSizeIcon)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.Icon13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Icon13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffers.GetData(), sizeof(FConstantBufferDebugIcon) * Buffers.Num()); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.Icon13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::UpdateTextureIcon(IconType type)
{
    DeviceContext->PSSetShaderResources(0, 1, &Resources.IconTextures[type]->TextureSRV);
    DeviceContext->PSSetSamplers(0, 1, &Resources.IconTextures[type]->SamplerState);
}

void FEditorRenderPass::RenderArrows()
{
    // XYZ한번. Z는 중복으로 적용
    const float ArrowScale = 3;

    ShaderManager->SetVertexShaderAndInputLayout(ShaderNameArrow, DeviceContext);
    ShaderManager->SetPixelShader(ShaderNameArrow, DeviceContext);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Arrow.Vertex, &Resources.Primitives.Arrow.VertexStride, &offset);
    DeviceContext->IASetIndexBuffer(Resources.Primitives.Arrow.Index, DXGI_FORMAT_R32_UINT, 0);

    PrepareConstantbufferArrow();
    for (UDirectionalLightComponent* DLightComp : Resources.Components.DirLight)
    {
        FConstantBufferDebugArrow buf;
        buf.Position = DLightComp->GetWorldLocation();
        buf.ArrowScaleXYZ = ArrowScale;
        buf.Direction = DLightComp->GetForwardVector();
        buf.ArrowScaleZ = ArrowScale;
        buf.Color = DLightComp->GetLightColor();
        UdpateConstantbufferArrow(buf);
        DeviceContext->DrawIndexed(Resources.Primitives.Arrow.NumIndices, 0, 0);

    }
    for (USpotLightComponent* SLightComp : Resources.Components.SpotLight)
    {
        FConstantBufferDebugArrow buf;
        buf.Position = SLightComp->GetWorldLocation();
        buf.ArrowScaleXYZ = ArrowScale;
        buf.Direction = SLightComp->GetForwardVector();
        buf.ArrowScaleZ = ArrowScale;
        buf.Color = SLightComp->GetLightColor();
        UdpateConstantbufferArrow(buf);
        DeviceContext->DrawIndexed(Resources.Primitives.Arrow.NumIndices, 0, 0);

    }
}

void FEditorRenderPass::PrepareConstantbufferArrow()
{
    if (Resources.ConstantBuffers.Arrow13)
    {
        DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Arrow13);
    }
}

void FEditorRenderPass::UdpateConstantbufferArrow(FConstantBufferDebugArrow Buffer)
{
    if (Resources.ConstantBuffers.Arrow13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        DeviceContext->Map(Resources.ConstantBuffers.Arrow13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(FConstantBufferDebugArrow)); // TArray이니까 실제 값을 받아와야함
        DeviceContext->Unmap(Resources.ConstantBuffers.Arrow13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderPass::UpdateShaders()
{

    // 디버그용
    //// 이전의 shader를 저장
    //ID3D11VertexShader* PreviousVertexShaderMesh = ShaderManager->GetVertexShaderByKey(ShaderNameCone);
    //ID3D11InputLayout* PreviousInputLayoutMesh = ShaderManager->GetInputLayoutByKey(ShaderNameCone);
    //ID3D11PixelShader* PreviousPixelShaderMesh = ShaderManager->GetPixelShaderByKey(ShaderNameCone);

    // AABB
    D3D11_INPUT_ELEMENT_DESC PositionOnlyLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    ShaderManager->ReloadModifiedShaders(ShaderNameCone, ShaderPath, "coneVS",
        PositionOnlyLayout, ARRAYSIZE(PositionOnlyLayout), defines,
        ShaderNameCone, ShaderPath, "conePS", defines);

}
