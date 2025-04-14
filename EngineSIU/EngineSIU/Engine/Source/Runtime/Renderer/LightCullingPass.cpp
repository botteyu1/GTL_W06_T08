#include "LightCullingPass.h"
#include "UObject/UObjectIterator.h"
#include "Components/Light/PointLightComponent.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"

FLightCullingPass::FLightCullingPass()
{
}

FLightCullingPass::~FLightCullingPass()
{
}

void FLightCullingPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    CreateShaders();
    CreateBuffers();
}

void FLightCullingPass::PrepareRender()
{
}

void FLightCullingPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    UpdateLightList();
    UpdateScreenInfoBuffer(Viewport);

    Graphics->DeviceContext->CSSetShader(ComputeShader, nullptr, 0);

    ID3D11ShaderResourceView* srvs[] = { LightListSRV };
    Graphics->DeviceContext->CSSetShaderResources(0, 1, srvs);

    UINT clearValue[4] = { 0, 0, 0, 0 };  // 0으로 초기화
    Graphics->DeviceContext->ClearUnorderedAccessViewUint(TileLightListUAV, clearValue);

    Graphics->DeviceContext->CSSetUnorderedAccessViews(1, 1, &TileLightListUAV, nullptr);

    // 디스패치: 각 타일 하나당 한 스레드 그룹
    Graphics->DeviceContext->Dispatch(NumTileWidth, NumTileHeight, 1);

    // 언바인딩
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    Graphics->DeviceContext->CSSetUnorderedAccessViews(1, 1, nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    Graphics->DeviceContext->CSSetShaderResources(0, 1, nullSRV);

    // 시각화 (Full-screen quad 렌더링)
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ShaderManager->SetVertexShader(ShaderName, Graphics->DeviceContext);
    ShaderManager->SetPixelShader(ShaderName, Graphics->DeviceContext);
    Graphics->DeviceContext->PSSetShaderResources(1, 1, &TileLightListSRV); // t1번 슬롯에 SRV 바인딩
    Graphics->DeviceContext->Draw(4, 0);

    Graphics->DeviceContext->PSSetShaderResources(1, 1, nullSRV);
}

void FLightCullingPass::ClearRenderArr()
{
}

HRESULT FLightCullingPass::CreateShaders()
{
    const D3D_SHADER_MACRO defines[] =
    {
        "MAX_NUM_GLOBAL_LIGHT", "256",
        NULL, NULL
    };

    HRESULT hr = ShaderManager->AddVertexShader(ShaderName, L"Shaders/LightCulling/LightCulling.hlsl", "mainVS", defines);
    if (FAILED(hr))
    {
        return hr;
    }
    hr = ShaderManager->AddPixelShader(ShaderName, L"Shaders/LightCulling/LightCulling.hlsl", "mainPS", defines);
    if (FAILED(hr))
    {
        return hr;
    }


    hr = ShaderManager->AddComputeShader(ShaderName, L"Shaders/LightCulling/LightCulling.hlsl", "mainCS", defines);
    if (FAILED(hr))
    {
        return hr;
    }
    VisualizeVertexShader = ShaderManager->GetVertexShaderByKey(ShaderName);
    VisualizePixelShader = ShaderManager->GetPixelShaderByKey(ShaderName);
    ComputeShader = ShaderManager->GetComputeShaderByKey(ShaderName);

    return hr;
}

void FLightCullingPass::CreateBuffers()
{
    CreateGlobalLightList();
    CreateTileLightList();
    CreateScreebInfoBuffer();
}

HRESULT FLightCullingPass::CreateScreebInfoBuffer()
{
    return BufferManager->CreateBufferGeneric<ScreenInfo>("ScreenInfo", nullptr, sizeof(ScreenInfo), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void FLightCullingPass::UpdateScreenInfoBuffer(std::shared_ptr<FEditorViewportClient> Viewport)
{
    ScreenInfo info;
    info.ProjInv = FMatrix::Inverse(Viewport->GetProjectionMatrix());
    info.ViewMatrix = Viewport->GetViewMatrix();
    info.NumTileWidth = NumTileWidth;
    info.NumTileHeight = NumTileHeight;
    info.TileSize = TileSize;
    info.ScreenWidth = Graphics->screenWidth;
    info.ScreenHeight = Graphics->screenHeight;
    BufferManager->UpdateConstantBuffer("ScreenInfo", &info);
    BufferManager->BindConstantBuffer("ScreenInfo", 13, EShaderStage::Compute);
}

HRESULT FLightCullingPass::CreateGlobalLightList()
{
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = sizeof(FPointLightData) * MaxNumPointLight;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.StructureByteStride = sizeof(FPointLightData);
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HRESULT hr;
    hr = Graphics->Device->CreateBuffer(&bufDesc, nullptr, &GlobalLightListBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.NumElements = MaxNumPointLight;

    hr = Graphics->Device->CreateShaderResourceView(GlobalLightListBuffer, &srvDesc, &LightListSRV);
    if (FAILED(hr))
    {
        return hr;
    }
    return hr;
}

void FLightCullingPass::UpdateLightList()
{
    TArray<FPointLightData> PointLightDatas;
    PointLightDatas.Reserve(MaxNumPointLight);
    for (const auto plight : TObjectRange<UPointLightComponent>())
    {
        FPointLightData data;
        data.Position = plight->GetWorldLocation();
        data.Radius = plight->GetAttenuationRadius();

        PointLightDatas.Add(data);
        if (PointLightDatas.Num() > MaxNumPointLight)
        {
            break;
        }
    }

    Graphics->DeviceContext->UpdateSubresource(
        GlobalLightListBuffer, 0, nullptr,
        PointLightDatas.GetData(),
        0, 0
    );
}

HRESULT FLightCullingPass::CreateTileLightList()
{
    NumTileWidth = ceil(Graphics->screenWidth / (float)TileSize);
    NumTileHeight = ceil(Graphics->screenHeight / (float)TileSize);
    
    uint32 tileCount = NumTileWidth * NumTileHeight;
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.ByteWidth = sizeof(FTileLightIndex) * tileCount;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.StructureByteStride = sizeof(FTileLightIndex);
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HRESULT hr;
    hr = Graphics->Device->CreateBuffer(&bufDesc, nullptr, &TileLightListBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = tileCount;

    hr = Graphics->Device->CreateUnorderedAccessView(TileLightListBuffer, &uavDesc, &TileLightListUAV);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.NumElements = tileCount;

    hr = Graphics->Device->CreateShaderResourceView(TileLightListBuffer, &srvDesc, &TileLightListSRV);
    if (FAILED(hr))
    {
        return hr;
    }

}


