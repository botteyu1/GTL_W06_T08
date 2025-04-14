#include "SpotLightComponent.h"

#include "UObject/Casts.h"

USpotLightComponent::USpotLightComponent()
{
    LightData.Intensity = Intensity;
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);

    LightData.Direction = GetForwardVector();
    LightData.Falloff = 1.0f;
    LightData.AttenuationRadius = 1.0f;
    LightData.InnerConeAngle = 0.0f;
    LightData.OuterConeAngle = 0.0f;
}

USpotLightComponent::~USpotLightComponent() = default;

UObject* USpotLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->LightData = LightData;
    return NewComponent;
}

void USpotLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightData_Color"), LightData.Color.ToString());
    OutProperties.Add(TEXT("LightData_Intensity"), FString::Printf(TEXT("%f"), LightData.Intensity));
    OutProperties.Add(TEXT("LightData_Falloff"), FString::Printf(TEXT("%f"), LightData.Falloff));
    OutProperties.Add(TEXT("LightData_AttenuationRadius"), FString::Printf(TEXT("%f"), LightData.AttenuationRadius));
    OutProperties.Add(TEXT("LightData_InnerConeAngle"), FString::Printf(TEXT("%f"), LightData.InnerConeAngle));
    OutProperties.Add(TEXT("LightData_OuterConeAngle"), FString::Printf(TEXT("%f"), LightData.OuterConeAngle));
    OutProperties.Add(TEXT("LightData_Direction"), LightData.Direction.ToString());
    OutProperties.Add(TEXT("LightData_Position"), LightData.Position.ToString());
    
}

void USpotLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("LightData_Color"));
    if (TempStr)
    {
        LightData.Color.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Intensity"));
    if (TempStr)
    {
        LightData.Intensity = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Falloff"));
    if (TempStr)
    {
        LightData.Falloff = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_AttenuationRadius"));
    if (TempStr)
    {
        LightData.AttenuationRadius = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_InnerConeAngle"));
    if (TempStr)
    {
        LightData.InnerConeAngle = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_OuterConeAngle"));
    if (TempStr)
    {
        LightData.OuterConeAngle = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Direction"));
    if (TempStr)
    {
        LightData.Direction.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Position"));
    if (TempStr)
    {
        LightData.Position.InitFromString(*TempStr);
    }
    
}

void USpotLightComponent::TickComponent(float DeltaTime)
{
    ULightComponent::TickComponent(DeltaTime);

    LightData.Direction = GetForwardVector();
    LightData.Position = GetWorldLocation();
}

void USpotLightComponent::SetInnerConeAngle(float InnerConeAngle)
{ 
    LightData.InnerConeAngle = InnerConeAngle; 
    LightData.OuterConeAngle = std::max(LightData.InnerConeAngle, LightData.OuterConeAngle);
}

void USpotLightComponent::SetOuterConeAngle(float OuterConeAngle)
{ 
    LightData.OuterConeAngle = OuterConeAngle; 
    LightData.InnerConeAngle = std::min(LightData.InnerConeAngle, LightData.OuterConeAngle);
}

void* USpotLightComponent::GetLightDefinition()
{
    return &LightData;
}
