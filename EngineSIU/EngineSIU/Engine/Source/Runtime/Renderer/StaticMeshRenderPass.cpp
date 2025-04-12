#include "StaticMeshRenderPass.h"

#include "EngineLoop.h"
#include "World/World.h"

#include "RendererHelpers.h"
#include "Math/JungleMath.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/StaticMeshComponent.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"

#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"

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

    std::string strDir = std::to_string(NUM_MAX_DIRLIGHT);
    std::string strPoint = std::to_string(NUM_MAX_POINTLIGHT);
    std::string strSpot = std::to_string(NUM_MAX_SPOTLIGHT);

    const D3D_SHADER_MACRO UberDefines[] =
    {
        { "NUM_MAX_DIRLIGHT",   strDir.c_str() },
        { "NUM_MAX_POINTLIGHT", strPoint.c_str() },
        { "NUM_MAX_SPOTLIGHT",  strSpot.c_str() },
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

void FStaticMeshRenderPass::ChangeViewMode(EViewModeIndex evi) const
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        UpdateLitUnlitConstant(1);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        UpdateLitUnlitConstant(0);
        break;
    }
}

// Uberlit.hlsl로 변경
void FStaticMeshRenderPass::SetUberShader(bool bValue)
{
	if (bValue)
	{
		VertexShader = ShaderManager->GetVertexShaderByKey(L"UberShaderVertex");

		PixelShader = ShaderManager->GetPixelShaderByKey(L"UberShaderPixel");

		InputLayout = ShaderManager->GetInputLayoutByKey(L"UberShaderVertex");
	}
	else
	{
		VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");

		PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShader");

		InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
	}
	IsUber = bValue;
}

void FStaticMeshRenderPass::UpdateShaders()
{
	// 이전의 shader를 저장
	ID3D11VertexShader* PreviousVertexShaderMesh = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
	ID3D11InputLayout* PreviousInputLayoutMesh = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
	ID3D11PixelShader* PreviousPixelShaderMesh = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShader");

	ID3D11VertexShader* PreviousVertexShaderUber = ShaderManager->GetVertexShaderByKey(L"UberShaderVertex");
	ID3D11PixelShader* PreviousPixelShaderUber = ShaderManager->GetPixelShaderByKey(L"UberShaderPixel");
	ID3D11InputLayout* PreviousInputLayoutUber = ShaderManager->GetInputLayoutByKey(L"UberShaderVertex");

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
	// Vertex Shader 생성 실패하면 그대로 종료
	if (FAILED(hr))
	{
		return;
	}
	hr = ShaderManager->AddPixelShader(L"StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS");
	// Vertex는 성공했지만 Pixel은 실패했으면, Vertex는 revert
	if (FAILED(hr))
	{
		ShaderManager->RemoveVertexShaderByKey(L"StaticMeshVertexShader");
		ShaderManager->RemoveInputLayoutByKey(L"StaticMeshVertexShader");

		ShaderManager->AddVertexShader(L"StaticMeshVertexShader", PreviousVertexShaderMesh);
		ShaderManager->AddInputLayout(L"UberShaderVertex", PreviousInputLayoutMesh);
	}
	// 이전의 Shader는 release
	else
	{
		PreviousVertexShaderMesh->Release();
		PreviousInputLayoutMesh->Release();
		PreviousPixelShaderMesh->Release();
	}

	std::string strDir = std::to_string(NUM_MAX_DIRLIGHT);
	std::string strPoint = std::to_string(NUM_MAX_POINTLIGHT);
	std::string strSpot = std::to_string(NUM_MAX_SPOTLIGHT);

	const D3D_SHADER_MACRO UberDefines[] =
	{
		{ "NUM_MAX_DIRLIGHT",   strDir.c_str() },
		{ "NUM_MAX_POINTLIGHT", strPoint.c_str() },
		{ "NUM_MAX_SPOTLIGHT",  strSpot.c_str() },
		{ NULL, NULL }
	};

	SetUberShader(IsUber);

	hr = ShaderManager->AddVertexShaderAndInputLayout(L"UberShaderVertex", L"Shaders/UberLit/UberLit.hlsl", "Uber_VS", StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc), UberDefines);
	// Vertex Shader 생성 실패하면 그대로 종료
	if (FAILED(hr))
	{
		return;
	}
	hr = ShaderManager->AddPixelShader(L"UberShaderPixel", L"Shaders/UberLit/UberLit.hlsl", "Uber_PS", UberDefines);
	// Vertex는 성공했지만 Pixel은 실패했으면, Vertex는 revert
	if (FAILED(hr))
	{
		ShaderManager->RemoveVertexShaderByKey(L"UberShaderVertex");
		ShaderManager->RemoveInputLayoutByKey(L"UberShaderVertex");

		ShaderManager->AddVertexShader(L"UberShaderVertex", PreviousVertexShaderUber);
		ShaderManager->AddInputLayout(L"UberShaderVertex", PreviousInputLayoutUber);
	}
	// 이전의 Shader는 release
	else
	{
		PreviousVertexShaderUber->Release();
		PreviousInputLayoutUber->Release();
		PreviousPixelShaderUber->Release();
	}

	SetUberShader(IsUber);
}


void FStaticMeshRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
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
    Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &PerObjectBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &CameraConstantBuffer);

    TArray<FString> PSBufferKeys = {
                                  TEXT("FCameraConstantBuffer"),
                                  TEXT("FSceneLightBuffer"),
                                  TEXT("FMaterialConstants"),
                                  TEXT("FLitUnlitConstants"),
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

void FStaticMeshRenderPass::UpdateLitUnlitConstant(int isLit) const
{
    FLitUnlitConstants Data;
    Data.isLit = isLit;
    BufferManager->UpdateConstantBuffer(TEXT("FLitUnlitConstants"), Data);
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

