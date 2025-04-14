#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
{
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

void UPointLightComponent::AddScale(FVector InAddValue)
{
    float delta = InAddValue.X + InAddValue.Y + InAddValue.Z;

    LightData.AttenuationRadius = std::max(0.1f, LightData.AttenuationRadius + delta);
}
void* UPointLightComponent::GetLightDefinition()
{
    return &LightData;
}
