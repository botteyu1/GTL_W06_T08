#include "DirectionalLightComponent.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    LightData.bVisible = bVisible;
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);
    LightData.Intensity = Intensity;
    LightData.Direction = this->GetForwardVector();
}

void UDirectionalLightComponent::TickComponent(float DeltaTime)
{
    ULightComponent::TickComponent(DeltaTime);

    LightData.Direction = GetWorldRotation();
}

void* UDirectionalLightComponent::GetLightDefinition()
{
    return &LightData;
}
