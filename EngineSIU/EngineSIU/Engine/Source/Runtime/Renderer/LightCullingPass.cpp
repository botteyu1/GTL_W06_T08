#include "LightCullingPass.h"
#include "UObject/UObjectIterator.h"
#include "Components/Light/PointLightComponent.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"

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
    CreateBlendState();
}

void FLightCullingPass::PrepareRender()
{
}

void FLightCullingPass::RenderHeatmap(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    if (!(Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_LightHeatMap))) return;

    PrepareBlendState();
    // 시각화 (Full-screen quad 렌더링)
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ShaderManager->SetVertexShader(ShaderNameVS, Graphics->DeviceContext);
    ShaderManager->SetPixelShader(ShaderNamePS, Graphics->DeviceContext);
    UpdateScreenInfoBuffer(Viewport);
    Graphics->DeviceContext->Draw(4, 0);

}

void FLightCullingPass::CullPointLight(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };

    ReloadShaders();

    Graphics->DeviceContext->PSSetShaderResources(16, 1, nullSRV); // t17번 슬롯에 SRV 바인딩
    Graphics->DeviceContext->PSSetShaderResources(17, 1, nullSRV); // t17번 슬롯에 SRV 바인딩

    UpdateLightList();
    UpdateScreenInfoBuffer(Viewport);
    Graphics->DeviceContext->CSSetShader(ComputeShader, nullptr, 0);

    ID3D11ShaderResourceView* srvs[] = { LightListSRV };
    Graphics->DeviceContext->CSSetShaderResources(0, 1, srvs);

    UINT clearValue[4] = { 0, 0, 0, 0 };  // 0으로 초기화
    Graphics->DeviceContext->ClearUnorderedAccessViewUint(TileLightListUAV, clearValue);

    Graphics->DeviceContext->CSSetUnorderedAccessViews(1, 1, &TileLightListUAV, nullptr);

    BufferManager->BindConstantBuffer("ScreenInfo", 10, EShaderStage::Compute);
    // 디스패치: 각 타일 하나당 한 스레드 그룹
    Graphics->DeviceContext->Dispatch(NumTileWidth, NumTileHeight, 1);

    // 언바인딩
    Graphics->DeviceContext->CSSetUnorderedAccessViews(1, 1, nullUAV, nullptr);

    Graphics->DeviceContext->CSSetShaderResources(0, 1, nullSRV);

    // static mesh를 위해서 미리 바인딩
    // -> staticMeshRenderPass에서 해줘야함
    Graphics->DeviceContext->PSSetShaderResources(16, 1, &LightListSRV); // t17번 슬롯에 SRV 바인딩
    Graphics->DeviceContext->PSSetShaderResources(17, 1, &TileLightListSRV); // t17번 슬롯에 SRV 바인딩
    Graphics->DeviceContext->VSSetShaderResources(16, 1, &LightListSRV); // t17번 슬롯에 SRV 바인딩
    Graphics->DeviceContext->VSSetShaderResources(17, 1, &TileLightListSRV); // t17번 슬롯에 SRV 바인딩
    BufferManager->BindConstantBuffer("ScreenInfo", 10, EShaderStage::Pixel);
    BufferManager->BindConstantBuffer("ScreenInfo", 10, EShaderStage::Vertex);

}

void FLightCullingPass::ClearRenderArr()
{
}

void FLightCullingPass::OnResize(HWND hWnd)
{
    ReleaseTileLightList();
    CreateTileLightList(Graphics);
}

void FLightCullingPass::CreateBlendState()
{
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    Graphics->Device->CreateBlendState(&blendDesc, &BlendState);



    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE; // 중요!
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    Graphics->Device->CreateDepthStencilState(&dsDesc, &DepthState);
}

void FLightCullingPass::PrepareBlendState()
{
    // 사용 시
    float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    Graphics->DeviceContext->OMSetBlendState(BlendState, blendFactor, 0xffffffff);

    Graphics->DeviceContext->OMSetDepthStencilState(DepthState, 0);
}

HRESULT FLightCullingPass::CreateShaders()
{


    HRESULT hr = ShaderManager->AddVertexShader(ShaderNameVS, L"Shaders/LightCulling/LightCulling.hlsl", "mainVS", LightCullingDefines);
    if (FAILED(hr))
    {
        return hr;
    }
    hr = ShaderManager->AddPixelShader(ShaderNamePS, L"Shaders/LightCulling/LightCulling.hlsl", "mainPS", LightCullingDefines);
    if (FAILED(hr))
    {
        return hr;
    }


    hr = ShaderManager->AddComputeShader(ShaderNameCS, L"Shaders/LightCulling/LightCulling.hlsl", "mainCS", LightCullingDefines);
    if (FAILED(hr))
    {
        return hr;
    }
    VisualizeVertexShader = ShaderManager->GetVertexShaderByKey(ShaderNameVS);
    VisualizePixelShader = ShaderManager->GetPixelShaderByKey(ShaderNamePS);
    ComputeShader = ShaderManager->GetComputeShaderByKey(ShaderNameCS);

    return hr;
}

HRESULT FLightCullingPass::ReloadShaders()
{

    HRESULT hr = ShaderManager->ReloadModifiedShaders(ShaderNameVS, L"Shaders/LightCulling/LightCulling.hlsl", "mainVS",
        nullptr, 0, LightCullingDefines, ShaderNamePS, L"Shaders/LightCulling/LightCulling.hlsl", "mainPS", LightCullingDefines);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = ShaderManager->ReloadModifiedComputeShader(ShaderNameCS, L"Shaders/LightCulling/LightCulling.hlsl", "mainCS", LightCullingDefines);
    if (FAILED(hr))
    {
        return hr;
    }
    VisualizeVertexShader = ShaderManager->GetVertexShaderByKey(ShaderNameVS);
    VisualizePixelShader = ShaderManager->GetPixelShaderByKey(ShaderNamePS);
    ComputeShader = ShaderManager->GetComputeShaderByKey(ShaderNameCS);

    return hr;
}

void FLightCullingPass::CreateBuffers()
{
    CreateGlobalLightList();
    CreateTileLightList(Graphics);
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
    info.ViewInv = FMatrix::Inverse(info.ViewMatrix);
    info.NumTileWidth = NumTileWidth;
    info.NumTileHeight = NumTileHeight;
    info.TileSize = TileSize;
    info.ScreenWidth = Viewport->GetViewport()->GetViewport().Width;
    info.ScreenHeight = Viewport->GetViewport()->GetViewport().Height;
    info.ScreenTopPadding = Viewport->GetViewport()->GetViewport().TopLeftY;
    info.MaxNumPointLight = MaxNumPointLight;
    BufferManager->UpdateConstantBuffer("ScreenInfo", info);
    //D3D11_MAPPED_SUBRESOURCE mappedResource;
    //HRESULT hr = DXDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    //if (FAILED(hr))
    //{
    //    UE_LOG(LogLevel::Error, TEXT("Buffer Map 실패, HRESULT: 0x%X"), hr);
    //    return;
    //}

    //memcpy(mappedResource.pData, &data, sizeof(T));
    //DXDeviceContext->Unmap(buffer, 0);

}

HRESULT FLightCullingPass::CreateGlobalLightList()
{
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = sizeof(FPointLight) * MaxNumPointLight;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.StructureByteStride = sizeof(FPointLight);
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
    TArray<FPointLight> PointLightDatas;
    PointLightDatas.Reserve(MaxNumPointLight);
    for (const auto plight : TObjectRange<UPointLightComponent>())
    {
        FPointLight data;
        data.Position = plight->GetWorldLocation();
        data.AttenuationRadius = plight->GetAttenuationRadius();
        data.Intensity = plight->GetIntensity();
        data.Color = plight->GetLightColor().rgb();
        data.Falloff = plight->GetFalloff();

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

HRESULT FLightCullingPass::CreateTileLightList(FGraphicsDevice* Graphics)
{
    FSlateRect Padding = FSlateRect(0, 0, 0, 0);
    // TODO: 다른 좋은 이름
    FVector2D AnchorMax = FVector2D(1.f, 1);
    FVector2D AnchorMin = FVector2D(0, 0);
    FSlateRect Rect = FSlateRect(
        FEngineLoop::GraphicDevice.ScreenWidth * AnchorMin.X + Padding.Left,
        FEngineLoop::GraphicDevice.ScreenHeight * AnchorMin.Y + Padding.Top,
        (FEngineLoop::GraphicDevice.ScreenWidth * AnchorMax.X) - Padding.Right,
        (FEngineLoop::GraphicDevice.ScreenHeight * AnchorMax.Y) - Padding.Bottom);
     
    NumTileWidth = ceil(Rect.GetWidth() / (float)TileSize);
    NumTileHeight = ceil(Rect.GetHeight() / (float)TileSize);
    
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

void FLightCullingPass::ReleaseTileLightList()
{
    BufferManager->SafeRelease(TileLightListUAV);
    BufferManager->SafeRelease(TileLightListSRV);
    BufferManager->SafeRelease(TileLightListBuffer);
}


