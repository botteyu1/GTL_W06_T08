#include "StaticMeshRenderPass.h"

#include "EngineLoop.h"
#include "World/World.h"

#include "RendererHelpers.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/StaticMeshComponent.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"
#include "Editor/LevelEditor/SLevelEditor.h"

#include "UnrealEd/EditorViewportClient.h"
#include "LightCullingPass.h"

FStaticMeshRenderPass::FStaticMeshRenderPass()
    : VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , Stride(0)
    , BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FStaticMeshRenderPass::~FStaticMeshRenderPass()
{
    ReleaseShader();
    if (ShaderManager)
    {
        delete ShaderManager;
        ShaderManager = nullptr;
    }
}

HRESULT FStaticMeshRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    D3D11_INPUT_ELEMENT_DESC TextureLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Stride = sizeof(FStaticMeshVertex);

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"StaticMeshVertexShader", L"Shaders/StaticMeshVertexShader.hlsl", "mainVS", StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc));
	if (FAILED(hr))
	{
		return hr;
	}
    hr = ShaderManager->AddPixelShader(L"StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS");
	if (FAILED(hr))
	{
		return hr;
	}
    VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");

    PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShader");

    InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");

    // refactoring here
    std::string strDir = std::to_string(NUM_MAX_DIRLIGHT);
    std::string strPoint = std::to_string(NUM_MAX_POINTLIGHT);
    std::string strSpot = std::to_string(NUM_MAX_SPOTLIGHT);
    std::string strGouraud = std::to_string(1);
    std::string strLambert = std::to_string(0);
    std::string strPhong = std::to_string(0);
    const D3D_SHADER_MACRO UberDefines[] =
    {
        { "NUM_MAX_DIRLIGHT",   strDir.c_str() },
        { "NUM_MAX_POINTLIGHT", strPoint.c_str() },
        { "NUM_MAX_SPOTLIGHT",  strSpot.c_str() },
        {"LIGHTING_MODEL_GOURAUD", strGouraud.c_str() },
        {"LIGHTING_MODEL_LAMBERT", strLambert.c_str() },
        {"LIGHTING_MODEL_PHONG", strPhong.c_str() },
        {"MAX_NUM_GLOBAL_LIGHT", MaxNumPointLightChar},
        {"MAX_NUM_INDICES_PER_TILE", MaxNumPointLightPerTileChar},
        { NULL, NULL }
    };

    hr = ShaderManager->AddVertexShaderAndInputLayout(L"UberShaderVertex", L"Shaders/UberLit/UberLit.hlsl", "Uber_VS", StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), UberDefines);
	if (FAILED(hr))
	{
		return hr;
	}
    hr = ShaderManager->AddPixelShader(L"UberShaderPixel", L"Shaders/UberLit/UberLit.hlsl", "Uber_PS", UberDefines);
	if (FAILED(hr))
	{
		return hr;
	}
}
void FStaticMeshRenderPass::ReleaseShader()
{

}

void FStaticMeshRenderPass::ChangeViewMode(EViewModeIndex evi)
{
    switch (evi)
    {
    case VMI_LitGouraud:
        UpdateShaders(1, 0, 0, true);
        UpdateFlag(FStaticMeshRenderPass::Flag::Lit);
        break;
    case VMI_LitLambert:
        UpdateShaders(0, 1, 0, true);
        UpdateFlag(FStaticMeshRenderPass::Flag::Lit);
        break;
    case VMI_LitBlinnPhong:
        UpdateShaders(0, 0, 1, true);
        UpdateFlag(FStaticMeshRenderPass::Flag::Lit);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        UpdateShaders(0, 0, 0, true);
        UpdateFlag(FStaticMeshRenderPass::Flag::Unlit);
        break;
    case VMI_WorldNormal:
        UpdateFlag(FStaticMeshRenderPass::Flag::Normal);
        break;
    case VMI_WorldPos:
        UpdateFlag(FStaticMeshRenderPass::Flag::Position);
        break;
    case VMI_SceneDepth:
        UpdateFlag(FStaticMeshRenderPass::Flag::Depth);
        break;
    case VMI_Attenuation:
        UpdateShaders(0, 0, 1, true);
        UpdateFlag(FStaticMeshRenderPass::Flag::Attenuation);
        break;
    }
}

// Uberlit.hlsl로 변경
bool FStaticMeshRenderPass::SetUberShader(bool bValue)
{
	if (bValue)
	{
		ID3D11VertexShader* NewVertexShader = ShaderManager->GetVertexShaderByKey(L"UberShaderVertex");

        ID3D11PixelShader* NewPixelShader = ShaderManager->GetPixelShaderByKey(L"UberShaderPixel");

        ID3D11InputLayout* NewInputLayout = ShaderManager->GetInputLayoutByKey(L"UberShaderVertex");

        if (NewVertexShader && NewPixelShader && NewInputLayout)
        {
            VertexShader = NewVertexShader;

            PixelShader = NewPixelShader;

            InputLayout = NewInputLayout;

            bIsUber = bValue;
            //UE_LOG(LogLevel::Display, "Changed to UberShader");

            return true;
        }
        // 실패. 하나 이상이 nullptr
        else
        {
            UE_LOG(LogLevel::Warning, "Failed to change shader. Target Shader is invalid.");
            bIsUber = !bValue;
            return false;
        }
	}
	else
	{
		ID3D11VertexShader* NewVertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");

        ID3D11PixelShader* NewPixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShader");

        ID3D11InputLayout* NewInputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");

        if (NewVertexShader && NewPixelShader && NewInputLayout)
        {
            VertexShader = NewVertexShader;

            PixelShader = NewPixelShader;

            InputLayout = NewInputLayout;

	        bIsUber = bValue;
            //UE_LOG(LogLevel::Display, "Changed to StaticMeshShader");
            return true;
        }
        // 실패. 하나 이상이 nullptr
        else
        {
            UE_LOG(LogLevel::Warning, "Failed to change shader. Target Shader is invalid.");
            bIsUber = !bValue;
            return false;
        }
	}
}

void FStaticMeshRenderPass::UpdateShaders(int32 GouraudFlag, int32 LambertFlag, int32 PhongFlag, bool ForceReload)
{
	D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};



    if (ForceReload)
    {
        ShaderManager->ReloadShaders(L"StaticMeshVertexShader", L"Shaders/StaticMeshVertexShader.hlsl", "mainVS",
            StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), nullptr,
            L"StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", nullptr);
        SetUberShader(bIsUber);
    }
    else
    {
        if (ShaderManager->ReloadModifiedShaders(L"StaticMeshVertexShader", L"Shaders/StaticMeshVertexShader.hlsl", "mainVS",
            StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), nullptr,
            L"StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", nullptr))
        {
            SetUberShader(bIsUber);
        }

    }


    // refactoring here
	std::string strDir = std::to_string(NUM_MAX_DIRLIGHT);
	std::string strPoint = std::to_string(NUM_MAX_POINTLIGHT);
	std::string strSpot = std::to_string(NUM_MAX_SPOTLIGHT);
    std::string strGouraud = std::to_string(GouraudFlag);
    std::string strLambert = std::to_string(LambertFlag);
    std::string strPhong = std::to_string(PhongFlag);
    
	const D3D_SHADER_MACRO UberDefines[] =
	{
		{ "NUM_MAX_DIRLIGHT",   strDir.c_str() },
		{ "NUM_MAX_POINTLIGHT", strPoint.c_str() },
		{ "NUM_MAX_SPOTLIGHT",  strSpot.c_str() },
	    {"LIGHTING_MODEL_GOURAUD", strGouraud.c_str() },
	    {"LIGHTING_MODEL_LAMBERT", strLambert.c_str() },
	    {"LIGHTING_MODEL_PHONG", strPhong.c_str() },
        {"MAX_NUM_GLOBAL_LIGHT", MaxNumPointLightChar},
        {"MAX_NUM_INDICES_PER_TILE", MaxNumPointLightPerTileChar},
		{ NULL, NULL }
	};

    if (ForceReload)
    {
        ShaderManager->ReloadShaders(L"UberShaderVertex", L"Shaders/UberLit/UberLit.hlsl", "Uber_VS",
            StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), UberDefines,
            L"UberShaderPixel", L"Shaders/UberLit/UberLit.hlsl", "Uber_PS", UberDefines);
        SetUberShader(bIsUber);
    }
    else
    {
        if (ShaderManager->ReloadModifiedShaders(L"UberShaderVertex", L"Shaders/UberLit/UberLit.hlsl", "Uber_VS",
            StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), UberDefines,
            L"UberShaderPixel", L"Shaders/UberLit/UberLit.hlsl", "Uber_PS", UberDefines))
        {
            SetUberShader(bIsUber);
        }
    }
	
}


void FStaticMeshRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
    UpdateShaders(0, 0, 1, true);
    bIsUber = true;
    SetUberShader(true);
}

void FStaticMeshRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshObjs.Add(iter);
        }
    }
}

void FStaticMeshRenderPass::PrepareRenderState() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    // 상수 버퍼 바인딩 예시
    ID3D11Buffer* PerObjectBuffer = BufferManager->GetConstantBuffer(TEXT("FPerObjectConstantBuffer"));
    ID3D11Buffer* CameraConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FCameraConstantBuffer"));
    ID3D11Buffer* LightBuffer = BufferManager->GetConstantBuffer(TEXT("FSceneLightBuffer"));
    ID3D11Buffer* MaterialBuffer = BufferManager->GetConstantBuffer(TEXT("FMaterialConstants"));
    ID3D11Buffer* TextureBuffer = BufferManager->GetConstantBuffer(TEXT("FTextureConstants"));
    
    Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &PerObjectBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &CameraConstantBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(2, 1, &LightBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &MaterialBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(6, 1, &TextureBuffer);

    TArray<FString> PSBufferKeys = {
                                  TEXT("FCameraConstantBuffer"),
                                  TEXT("FSceneLightBuffer"),
                                  TEXT("FMaterialConstants"),
                                  TEXT("FFlagConstants"),
                                  TEXT("FSubMeshConstants"),
                                  TEXT("FTextureConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 1, EShaderStage::Pixel);
}

void FStaticMeshRenderPass::UpdatePerObjectConstant(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FVector4& UUIDColor, bool Selected) const
{
    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);
    FPerObjectConstantBuffer Data(Model, NormalMatrix, UUIDColor, Selected);
    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), Data);
   
}

void FStaticMeshRenderPass::UpdateFlag(FStaticMeshRenderPass::Flag InFlag) const
{
    FFlagConstants Data;
    Data.RenderFlag = static_cast<int>(InFlag);
    BufferManager->UpdateConstantBuffer(TEXT("FFlagConstants"), Data);
}


void FStaticMeshRenderPass::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &offset);
    if (RenderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (RenderData->MaterialSubsets.Num() == 0) {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int subMeshIndex = 0; subMeshIndex < RenderData->MaterialSubsets.Num(); subMeshIndex++) {

        int materialIndex = RenderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        FSubMeshConstants SubMeshData = (subMeshIndex == SelectedSubMeshIndex) ? FSubMeshConstants(true) : FSubMeshConstants(false);

        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        if (OverrideMaterials[materialIndex] != nullptr)
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[materialIndex]->GetMaterialInfo());
        else
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[materialIndex]->Material->GetMaterialInfo());

        uint64 startIndex = RenderData->MaterialSubsets[subMeshIndex].IndexStart;
        uint64 indexCount = RenderData->MaterialSubsets[subMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
    }
}


void FStaticMeshRenderPass::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FStaticMeshRenderPass::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FStaticMeshRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    if ((GetAsyncKeyState('P') & 0x0001) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
    {
        UpdateShaders(0,0,1, true);
        Sleep(100); // 여러번 눌리는걸 방지하기 위해서 6프레임동안 멈춤
    }

    if ((GetAsyncKeyState('O') & 0x0001) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
    {
        SetUberShader(!bIsUber);
        Sleep(100); // 여러번 눌리는걸 방지하기 위해서 6프레임동안 멈춤
    }
  
    if (bAutoUpdate)
    {
        EViewModeIndex mode = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient()->GetViewMode();
        switch (mode)
        {
        case VMI_LitGouraud:
            GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(1, 0, 0, false);
            break;
        case VMI_LitLambert:
            GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(0, 1, 0, false);
            break;
        case VMI_LitBlinnPhong:
            GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(0, 0, 1, false);
            break;
        default:
            GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(0, 0, 1, false);
            break;
        }
    }
    if (!(Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))) return;

    PrepareRenderState();

    for (UStaticMeshComponent* Comp : StaticMeshObjs) {
        if (!Comp || !Comp->GetStaticMesh()) continue;
        
        FMatrix Model = Comp->GetWorldMatrix();

        FVector4 UUIDColor = Comp->EncodeUUID() / 255.0f;

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();
        AActor* SelectedActor = Engine->GetSelectedActor();

        USceneComponent* TargetComponent = nullptr;
    
        if (SelectedComponent != nullptr)
        {
            TargetComponent = SelectedComponent;
        }
        else if (SelectedActor != nullptr)
        {
            TargetComponent = SelectedActor->GetRootComponent();
        }
        
        bool Selected = (Engine && TargetComponent == Comp);

        UpdatePerObjectConstant(Model, Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix(), UUIDColor, Selected);
        FCameraConstantBuffer CameraData(Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix(), Viewport->ViewTransformPerspective.GetLocation(), 0);
        BufferManager->UpdateConstantBuffer(TEXT("FCameraConstantBuffer"), CameraData);

        OBJ::FStaticMeshRenderData* RenderData = Comp->GetStaticMesh()->GetRenderData();

        if (RenderData == nullptr) continue;

        RenderPrimitive(RenderData, Comp->GetStaticMesh()->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());

        if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            FEngineLoop::PrimitiveDrawBatch.AddAABBToBatch(Comp->GetLocalBoundingBox(), Comp->GetWorldLocation(), Model);
        }
    }
}

void FStaticMeshRenderPass::ClearRenderArr()
{
    StaticMeshObjs.Empty();
}

