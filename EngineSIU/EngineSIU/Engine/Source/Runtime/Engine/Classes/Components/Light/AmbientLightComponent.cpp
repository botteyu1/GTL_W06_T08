#include "AmbientLightComponent.h"

UAmbientLightComponent::UAmbientLightComponent()
{
    LightData.Intensity = Intensity;
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);
}

void* UAmbientLightComponent::GetLightDefinition()
{
    return &LightData;
}
