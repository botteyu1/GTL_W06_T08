#include "ZPrepassRenderPass.h"
#include "StaticMeshRenderPass.h"
#include "UObject/UObjectIterator.h"
#include "Components/Light/PointLightComponent.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"
#include "BaseGizmos/GizmoBaseComponent.h"

FZPrepassRenderPassStaticMesh::FZPrepassRenderPassStaticMesh()
{
}

FZPrepassRenderPassStaticMesh::~FZPrepassRenderPassStaticMesh()
{
}

void FZPrepassRenderPassStaticMesh::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShaders();
    CreateBuffers();
    CreateState();
}

void FZPrepassRenderPassStaticMesh::PrepareRender()
{
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilStateTestWriteEnable, 0);
    Graphics->DeviceContext->OMSetBlendState(BlendOffState, nullptr, 0xFFFFFFFF);

    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, Graphics->DepthStencilView);  // depth-only render
    Graphics->DeviceContext->ClearDepthStencilView(Graphics->DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void FZPrepassRenderPassStaticMesh::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    //return;
    PrepareStaticMeshes();
    PrepareRender();
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0); // depth 전용 vertex shader
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);     // pixel shader 사용 안함
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    FZprepassPerSceneConstantBuffer SceneData (Viewport->GetViewMatrix() * Viewport->GetProjectionMatrix());
    BufferManager->UpdateConstantBuffer(TEXT("FZprepassPerSceneConstantBuffer"), SceneData);

    // 상수 버퍼 바인딩 예시
    ID3D11Buffer* ZprepassPerMeshConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FZprepassPerMeshConstantBuffer"));
    ID3D11Buffer* ZprepassPerSceneConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FZprepassPerSceneConstantBuffer"));

    TArray<FString> VSBufferKeys = {
                                  TEXT("FZprepassPerMeshConstantBuffer"),
                                  TEXT("FZprepassPerSceneConstantBuffer"),
    };

    BufferManager->BindConstantBuffers(VSBufferKeys, 0, EShaderStage::Vertex);
    
    for (UStaticMeshComponent* Comp : this->StaticMeshObjs) {
        if (!Comp || !Comp->GetStaticMesh()) continue;

        FMatrix Model = Comp->GetWorldMatrix();

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        FZprepassPerMeshConstantBuffer Data(Model);
        BufferManager->UpdateConstantBuffer(TEXT("FZprepassPerMeshConstantBuffer"), Data);
        

        OBJ::FStaticMeshRenderData* RenderData = Comp->GetStaticMesh()->GetRenderData();

        if (RenderData == nullptr) continue;

        UINT offset = 0;
        Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &offset);
        if (RenderData->IndexBuffer)
            Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        if (RenderData->MaterialSubsets.Num() == 0) {
            Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
            return;
        }

        for (int subMeshIndex = 0; subMeshIndex < RenderData->MaterialSubsets.Num(); subMeshIndex++) {
            uint64 startIndex = RenderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = RenderData->MaterialSubsets[subMeshIndex].IndexCount;
            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }

    }


    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilStateTestWriteEnable, 0);

    Graphics->DeviceContext->OMSetRenderTargets(2, Graphics->RTVs, Graphics->DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
}

void FZPrepassRenderPassStaticMesh::ClearRenderArr()
{
}

HRESULT FZPrepassRenderPassStaticMesh::CreateShaders()
{

    D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Stride = sizeof(FStaticMeshVertex);

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(ShaderNameVS, L"Shaders/ZPrepass/ZPrepass.hlsl", "mainVS",
        StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), nullptr);
    if (FAILED(hr))
    {
        return hr;
    }
    //hr = ShaderManager->AddPixelShader(ShaderNamePS, L"Shaders/ZPrepass/ZPrepass.hlsl", "mainPS", nullptr);
    //if (FAILED(hr))
    //{
    //    return hr;
    //}
    VertexShader = ShaderManager->GetVertexShaderByKey(ShaderNameVS);

    //PixelShader = ShaderManager->GetPixelShaderByKey(ShaderNamePS);

    InputLayout = ShaderManager->GetInputLayoutByKey(ShaderNameVS);
}

HRESULT FZPrepassRenderPassStaticMesh::ReloadShaders(bool ForceReload)
{
    D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    HRESULT hr;

    if (ForceReload)
    {
        hr = ShaderManager->ReloadVertexShader(ShaderNameVS, L"Shaders/ZPrepass/ZPrepass.hlsl", "mainVS", nullptr);
    }
    else
    {
        hr = ShaderManager->ReloadModifiedVertexShader(ShaderNameVS, L"Shaders/ZPrepass/ZPrepass.hlsl", "mainVS", nullptr);
    }
    if (FAILED(hr))
    {
        return hr;
    }

    VertexShader = ShaderManager->GetVertexShaderByKey(ShaderNameVS);

    //PixelShader = ShaderManager->GetPixelShaderByKey(ShaderNamePS);

    InputLayout = ShaderManager->GetInputLayoutByKey(ShaderNameVS);
}

void FZPrepassRenderPassStaticMesh::ReleaseShaders()
{

}

HRESULT FZPrepassRenderPassStaticMesh::CreateBuffers()
{
    HRESULT hr;

    // constant buffers
    hr = BufferManager->CreateBufferGeneric<FZprepassPerMeshConstantBuffer>
        ("FZprepassPerMeshConstantBuffer", nullptr, sizeof(FZprepassPerMeshConstantBuffer),
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    if (FAILED(hr))
    {
        return hr;
    }
    hr = BufferManager->CreateBufferGeneric<FZprepassPerSceneConstantBuffer>
        ("FZprepassPerSceneConstantBuffer", nullptr, sizeof(FZprepassPerSceneConstantBuffer),
            D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    if (FAILED(hr))
    {
        return hr;
    }

    return hr;

    //D3D11_TEXTURE2D_DESC depthDesc = {};
    //depthDesc.Width = Graphics->ScreenWidth;
    //depthDesc.Height = Graphics->ScreenHeight;
    //depthDesc.MipLevels = 1;
    //depthDesc.ArraySize = 1;

    //// Depth 전용 포맷 (Stencil 없음)
    //depthDesc.Format = DXGI_FORMAT_D32_FLOAT;  // 32-bit float depth only
    //depthDesc.SampleDesc.Count = 1;
    //depthDesc.Usage = D3D11_USAGE_DEFAULT;
    //depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // 나중에 SRV로도 가능
    //
    //hr = Graphics->Device->CreateTexture2D(&depthDesc, nullptr, &DepthMap);
    //if (FAILED(hr))
    //{
    //    return hr;
    //}

    //D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    //dsvDesc.Format = depthDesc.Format;
    //dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    //dsvDesc.Texture2D.MipSlice = 0;

    //hr = Graphics->Device->CreateDepthStencilView(DepthMap, &dsvDesc, &DepthView);
    //if (FAILED(hr))
    //{
    //    return hr;
    //}
    //return hr;
}

HRESULT FZPrepassRenderPassStaticMesh::CreateState()
{
    //DepthState = Graphics->DepthStencilStateTestWriteEnable;
    //return DepthState ? S_OK : S_FALSE;



    //D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    //dsDesc.DepthEnable = TRUE;
    //dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    //dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    //dsDesc.StencilEnable = FALSE;

    HRESULT hr;
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0; // Color write 모두 OFF

    hr = Graphics->Device->CreateBlendState(&blendDesc, &BlendOffState);
    if (FAILED(hr))
    {
        return hr;
    }
    return hr;
}

void FZPrepassRenderPassStaticMesh::PrepareStaticMeshes()
{
    StaticMeshObjs.Empty();

    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshObjs.Add(iter);
        }
    }
}
