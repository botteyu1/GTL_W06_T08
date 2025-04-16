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
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override {}
    virtual void ClearRenderArr() override;

    void RenderHeatmap(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CullPointLight(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void OnResize(HWND hWnd);

    const D3D_SHADER_MACRO LightCullingDefines[3] =
    {
        "MAX_NUM_GLOBAL_LIGHT", MaxNumPointLightChar,
        "MAX_NUM_INDICES_PER_TILE", MaxNumPointLightPerTileChar,
        NULL, NULL,
    };
private:
    void CreateBlendState();
    void PrepareBlendState();

    // GPU 내부에서 서로 주고받는 용도
    struct FTileLightIndex
    {
        uint32 LightCount;
        uint32 LightIndices[MaxNumPointLightPerTile];
    };

    // Tile 만들때 필요한 data
    struct ScreenInfo
    {
        FMatrix ProjInv;
        FMatrix ViewMatrix;
        FMatrix ViewInv;
        uint32 NumTileWidth;
        uint32 NumTileHeight;
        uint32 TileSize;
        uint32 ScreenWidth;
        uint32 ScreenHeight;
        uint32 ScreenTopPadding;
        uint32 MaxNumPointLight;
        uint32 pad1;
    };

    HRESULT CreateShaders();
    HRESULT ReloadShaders();
    const std::wstring ShaderNameVS = L"LightCullingShaderVS";
    const std::wstring ShaderNamePS = L"LightCullingShaderPS";
    const std::wstring ShaderNameCS = L"LightCullingShaderCS";
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
    HRESULT CreateTileLightList(FGraphicsDevice* Graphics);
    void ReleaseTileLightList();
    ID3D11Buffer* TileLightListBuffer;
    ID3D11UnorderedAccessView* TileLightListUAV;
    ID3D11ShaderResourceView* TileLightListSRV;

    ID3D11BlendState* BlendState;
    ID3D11DepthStencilState* DepthState;



    TArray<class UPointLightComponent*> PointLights;

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

};

