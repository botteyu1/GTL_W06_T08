#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"

#include "Components/Light/AmbientLightComponent.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"

#include "UObject/UObjectIterator.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FUpdateLightBufferPass::~FUpdateLightBufferPass()
{
}

void FUpdateLightBufferPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FUpdateLightBufferPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
            else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
            else if (UDirectionalLightComponent* DL = Cast<UDirectionalLightComponent>(iter))
            {
                this->DirectionalLight = DL;
            }
            else if (UAmbientLightComponent* AL = Cast<UAmbientLightComponent>(iter))
            {
                this->AmbientLight = AL;
            }
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // FLightBuffer LightBufferData = {};
    FSceneLightBuffer SceneLightBufferData = {};
    int LightCount = 0;

    // LightBufferData.GlobalAmbientLight = FVector4(0.1f, 0.1f, 0.1f, 1.f);
    if (this->AmbientLight)
    {
        SceneLightBufferData.AmbientLight = this->AmbientLight->GetLightData<FAmbientLight>();
    }
    else
    {
        FAmbientLight TempLight;
        TempLight.bVisible = true;
        TempLight.Color = FVector(0.1f, 0.1f, 0.1f);
        TempLight.Intensity = 1.0f;
        
        SceneLightBufferData.AmbientLight = TempLight; 
    }

    if (this->DirectionalLight)
    {
        SceneLightBufferData.DirectionalLight = this->DirectionalLight->GetLightData<FDirectionalLight>();
    }
    
    
    for (auto Light : PointLights)
    {
        // if (LightCount < MAX_LIGHTS)
        // {
        //     LightBufferData.gLights[LightCount] = Light->GetLightInfo();
        //     LightBufferData.gLights[LightCount].Position = Light->GetWorldLocation();
        //
        //     LightCount++;
        // }

        if (LightCount < MAX_POINT_LIGHT)
        {
            SceneLightBufferData.PointLight[LightCount] = Light->GetLightData<FPointLight>();
            LightCount++;
        }
    }
    SceneLightBufferData.NumPointLights = LightCount;
    
    // Reset count
    LightCount = 0;

    for (auto Light : SpotLights)
    {
        if (LightCount < MAX_LIGHTS)
        {
            //// 월드 변환 행렬 계산 (스케일 1로 가정)
            //FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });

            //FEngineLoop::PrimitiveDrawBatch.AddConeToBatch(Light->GetWorldLocation(), 100, Light->GetRange(), 140, {1,1,1,1}, Model);

            //FEngineLoop::PrimitiveDrawBatch.AddOBBToBatch(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
            // LightBufferData.gLights[LightCount] = Light->GetLightInfo();
            // LightBufferData.gLights[LightCount].Position = Light->GetWorldLocation();
            // LightBufferData.gLights[LightCount].Direction = Light->GetForwardVector();
            // LightBufferData.gLights[LightCount].Type = ELightType::SPOT_LIGHT;

            SceneLightBufferData.SpotLight[LightCount] = Light->GetLightData<FSpotLight>();
            LightCount++;
        }
    }
    SceneLightBufferData.NumSpotLights = LightCount;
    
    // LightBufferData.nLights = LightCount;

    // BufferManager->UpdateConstantBuffer(TEXT("FLightBuffer"), LightBufferData);
    BufferManager->UpdateConstantBuffer(TEXT("FSceneLightBuffer"), SceneLightBufferData);
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    PointLights.Empty();
    SpotLights.Empty();
}

void FUpdateLightBufferPass::UpdateLightBuffer(FLight Light) const
{

}
