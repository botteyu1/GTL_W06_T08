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

void UExponentialHeightFogComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("FogDensity"), FString::Printf(TEXT("%f"), FogDensity));
    OutProperties.Add(TEXT("FogHeightFalloff"), FString::Printf(TEXT("%f"), FogHeightFalloff));
    OutProperties.Add(TEXT("StartDistance"), FString::Printf(TEXT("%f"), StartDistance));
    OutProperties.Add(TEXT("FogCutoffDistance"), FString::Printf(TEXT("%f"), FogCutoffDistance));
    OutProperties.Add(TEXT("FogMaxOpacity"), FString::Printf(TEXT("%f"), FogMaxOpacity));
    //FVector4 Color = FVector4(FogInscatteringColor.R, FogInscatteringColor.G, FogInscatteringColor.B, FogInscatteringColor.A);
    
    OutProperties.Add(TEXT("FogInscatteringColor"), FString::Printf(TEXT("%s"), *FogInscatteringColor.ToString()));
}

void UExponentialHeightFogComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("FogDensity"));
    if (TempStr)
    {
        FogDensity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogHeightFalloff"));
    if (TempStr)
    {
        FogHeightFalloff = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("StartDistance"));
    if (TempStr)
    {
        StartDistance = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogCutoffDistance"));
    if (TempStr)
    {
        FogCutoffDistance = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogMaxOpacity"));
    if (TempStr)
    {
        FogMaxOpacity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogInscatteringColor"));
    if (TempStr)
    {
        // FVector4 Color;
        // Color.InitFromString(*TempStr);
        FogInscatteringColor = FLinearColor(*TempStr);
    }
}
