#pragma once

#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"
#include "Define.h"

class FLightCullingPass : public IRenderPass
{
public:
    FLightCullingPass();
    ~FLightCullingPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

private:
    // CPU -> GPU 전달용 scene의 전체 light를 담은 버퍼
    // TODO : 추후에 라이트 개수 늘어나면 어떻게해야할까..
    const uint32 MaxNumPointLight = 256; // define에서 바꾸기
    struct FPointLightData
    {
        FVector Position;
        float Radius;
    };

    // GPU 내부에서 서로 주고받는 용도
    struct FTileLightIndex
    {
        uint32 LightCount;
        uint32 LightIndices[31];
    };

    // Tile 만들때 필요한 data
    struct ScreenInfo
    {
        FMatrix ProjInv;
        FMatrix ViewMatrix;
        uint32 NumTileWidth;
        uint32 NumTileHeight;
        uint32 TileSize;
        uint32 ScreenWidth;
        uint32 ScreenHeight;
    };

    HRESULT CreateShaders();
    const std::wstring ShaderName = L"LightCullingShader";
    ID3D11VertexShader* VisualizeVertexShader;
    ID3D11PixelShader* VisualizePixelShader;
    ID3D11ComputeShader* ComputeShader;
    
    void CreateBuffers();

    const uint32 TileSize = 16;
    uint32 NumTileWidth, NumTileHeight;

    HRESULT CreateScreebInfoBuffer();
    void UpdateScreenInfoBuffer(std::shared_ptr<FEditorViewportClient> Viewport);

    //TArray<FPointLightData> PointLightDatas;
    HRESULT CreateGlobalLightList();
    void UpdateLightList();
    ID3D11Buffer* GlobalLightListBuffer;
    ID3D11ShaderResourceView* LightListSRV;
    HRESULT CreateTileLightList();
    ID3D11Buffer* TileLightListBuffer;
    ID3D11UnorderedAccessView* TileLightListUAV;

    


    TArray<class UPointLightComponent*> PointLights;

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

};

