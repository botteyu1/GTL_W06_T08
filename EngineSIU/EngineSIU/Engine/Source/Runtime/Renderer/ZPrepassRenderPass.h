#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"
#include "Define.h"

class FZPrepassRenderPassStaticMesh :
    public IRenderPass
{
public:
    FZPrepassRenderPassStaticMesh();
    ~FZPrepassRenderPassStaticMesh();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

private:
    // 내부에서만 쓰는 constant buffer 구조체
    struct FZprepassPerMeshConstantBuffer 
    {
        FMatrix Model;
    };
    struct FZprepassPerSceneConstantBuffer
    {
        FMatrix ViewProjection;
    };



    HRESULT CreateShaders();
    HRESULT ReloadShaders(bool ForceReload);
    void ReleaseShaders();

    HRESULT CreateBuffers();
    HRESULT CreateState();
    ID3D11BlendState* BlendOffState;

    // StaticMeshRenderPass와 동일한 과정 사용
    TArray<class UStaticMeshComponent*> StaticMeshObjs;

    void PrepareStaticMeshes();


    const std::wstring ShaderNameVS = L"ZPrepassStaticMeshShaderVS";
    const std::wstring ShaderNamePS = L"ZPrepassStaticMeshShaderPS";
    uint32 Stride;

    ID3D11VertexShader* VertexShader;
    //ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    //ID3D11Texture2D* DepthMap;
    //ID3D11DepthStencilView* DepthView;
    //ID3D11DepthStencilState* DepthState;

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
};

