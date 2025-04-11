#include "LightComponentBase.h"

ULightComponentBase::ULightComponentBase()
{
    Intensity = 1.0f;
    LightColor = FColor::White;
    bVisible = true;
}

float ULightComponentBase::GetIntensity() const
{
    return Intensity;
}

FColor ULightComponentBase::GetLightColor() const
{
    return LightColor;
}

bool ULightComponentBase::IsVisible() const
{
    return bVisible;
}
