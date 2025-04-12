#include "SpotLightComponent.h"
USpotLightComponent::USpotLightComponent()
{
    LightData.bVisible = bVisible;
    LightData.Intensity = Intensity;
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);

    LightData.Direction = GetForwardVector();
    LightData.Falloff = 1.0f;
    LightData.AttenuationRadius = 1.0f;
    LightData.InnerConeAngle = 0.0f;
    LightData.OuterConeAngle = 0.0f;
}

USpotLightComponent::~USpotLightComponent() = default;

void USpotLightComponent::TickComponent(float DeltaTime)
{
    ULightComponent::TickComponent(DeltaTime);

    LightData.Direction = GetForwardVector();
    LightData.Position = GetWorldLocation();
}

void* USpotLightComponent::GetLightDefinition()
{
    return &LightData;
}
