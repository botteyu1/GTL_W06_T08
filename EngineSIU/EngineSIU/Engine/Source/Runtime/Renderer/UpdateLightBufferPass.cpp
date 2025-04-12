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
    FSceneLightBuffer SceneLightBufferData = {};
    int LightCount = 0;
    
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
        SceneLightBufferData.DirectionalLight[0] = this->DirectionalLight->GetLightData<FDirectionalLight>();
    }
    
    
    for (auto Light : PointLights)
    {
        if (LightCount < NUM_MAX_POINTLIGHT)
        {
            SceneLightBufferData.PointLight[LightCount] = Light->GetLightData<FPointLight>();
            SceneLightBufferData.PointLight[LightCount].Color = FVector(Light->GetLightColor().R, Light->GetLightColor().G, Light->GetLightColor().B);
            SceneLightBufferData.PointLight[LightCount].Position = Light->GetWorldLocation();
            SceneLightBufferData.PointLight[LightCount].Intensity = Light->GetIntensity();
            SceneLightBufferData.PointLight[LightCount].bVisible = Light->IsVisible() ? 1 : 0;
            
            LightCount++;
        }
    }
    SceneLightBufferData.NumPointLights = LightCount;
    
    // Reset count
    LightCount = 0;

    for (auto Light : SpotLights)
    {
        if (LightCount < NUM_MAX_SPOTLIGHT)
        {
            SceneLightBufferData.SpotLight[LightCount] = Light->GetLightData<FSpotLight>();
            SceneLightBufferData.SpotLight[LightCount].Color = FVector(Light->GetLightColor().R, Light->GetLightColor().G, Light->GetLightColor().B);
            SceneLightBufferData.SpotLight[LightCount].Position = Light->GetWorldLocation();
            SceneLightBufferData.SpotLight[LightCount].Intensity = Light->GetIntensity();
            SceneLightBufferData.SpotLight[LightCount].bVisible = Light->IsVisible() ? 1 : 0;
            
            LightCount++;
        }
    }
    SceneLightBufferData.NumSpotLights = LightCount;
    
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
