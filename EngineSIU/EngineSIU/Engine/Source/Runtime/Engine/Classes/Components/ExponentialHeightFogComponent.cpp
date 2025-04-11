#include "ExponentialHeightFogComponent.h"
#include <UObject/Casts.h>

UExponentialHeightFogComponent::UExponentialHeightFogComponent(float Density, float HeightFalloff, float StartDist, float CutoffDist, float MaxOpacity)
    :FogDensity(Density), FogHeightFalloff(HeightFalloff), StartDistance(StartDist), FogCutoffDistance(CutoffDist), FogMaxOpacity(MaxOpacity)
{
    FogInscatteringColor = FLinearColor::White;
}

void UExponentialHeightFogComponent::SetFogDensity(float value)
{
    FogDensity = value;
}

void UExponentialHeightFogComponent::SetFogHeightFalloff(float value)
{
    FogHeightFalloff = value; 
}

void UExponentialHeightFogComponent::SetStartDistance(float value)
{
    StartDistance = value;
}

void UExponentialHeightFogComponent::SetFogCutoffDistance(float value)
{
    FogCutoffDistance = value;
}

void UExponentialHeightFogComponent::SetFogMaxOpacity(float value)
{
    FogMaxOpacity = value;
}

void UExponentialHeightFogComponent::SetFogColor(FLinearColor color)
{
    FogInscatteringColor = color;
}

UObject* UExponentialHeightFogComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->FogDensity = FogDensity;
    NewComponent->FogHeightFalloff = FogHeightFalloff;
    NewComponent->StartDistance = StartDistance;
    NewComponent->FogCutoffDistance = FogCutoffDistance;
    NewComponent->FogMaxOpacity = FogMaxOpacity;
    NewComponent->FogInscatteringColor = FogInscatteringColor;

    return NewComponent;
}
