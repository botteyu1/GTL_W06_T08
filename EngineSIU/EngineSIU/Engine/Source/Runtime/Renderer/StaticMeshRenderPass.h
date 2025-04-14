#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"

#include "Define.h"

class FDXDShaderManager;

class UWorld;

class UMaterial;

class FEditorViewportClient;

class UStaticMeshComponent;

struct FStaticMaterial;

class FStaticMeshRenderPass : public IRenderPass
{
    static enum class Flag
    {
        Unlit = 0,
        Lit = 1,
        Depth = 2,
        Position = 3,
        Normal = 4,
        Attenuation = 5,
    };
public:
    FStaticMeshRenderPass();
    
    ~FStaticMeshRenderPass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    
    virtual void PrepareRender() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

    void PrepareRenderState() const;
    
    void UpdatePerObjectConstant(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FVector4& UUIDColor, bool Selected) const;
  
    void UpdateFlag(FStaticMeshRenderPass::Flag InFlag) const;

    void RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const;
    
    void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const;

    void RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const;

    // Shader 관련 함수 (생성/해제 등)
    HRESULT CreateShader();

    void ReleaseShader();

    void ChangeViewMode(EViewModeIndex evi);

    bool SetUberShader(bool bValue);

    void UpdateShaders(int32 GouraudFlag = 1, int32 LambertFlag = 0, int32 PhongFlag = 0, bool ForceReload = false);

    bool IsUber() const { return bIsUber; }

    void SetAutoUpdate(bool bValue) { bAutoUpdate = bValue; }

private:
    TArray<UStaticMeshComponent*> StaticMeshObjs;

    ID3D11VertexShader* VertexShader;
    
    ID3D11PixelShader* PixelShader;
    
    ID3D11InputLayout* InputLayout;
    
    uint32 Stride;

    FDXDBufferManager* BufferManager;
    
    FGraphicsDevice* Graphics;
    
    FDXDShaderManager* ShaderManager;

    bool bIsUber = false;

    bool bAutoUpdate = false;
};
