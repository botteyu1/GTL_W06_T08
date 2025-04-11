#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
{
    LightData.bVisible = bVisible;
    LightData.Intensity = Intensity;
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);

    LightData.AttenuationRadius = 1.0f;
    LightData.Falloff = 1.0f;
}

UPointLightComponent::~UPointLightComponent() = default;

void UPointLightComponent::TickComponent(float DeltaTime)
{
    ULightComponent::TickComponent(DeltaTime);
}

void* UPointLightComponent::GetLightDefinition()
{
    return &LightData;
}
