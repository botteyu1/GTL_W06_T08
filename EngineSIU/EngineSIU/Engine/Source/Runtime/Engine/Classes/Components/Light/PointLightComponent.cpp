#include "PointLightComponent.h"

#include "UObject/Casts.h"

UObject* UPointLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->LightData = LightData;
    return NewComponent;
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightData_Color"), LightData.Color.ToString());
    OutProperties.Add(TEXT("LightData_Intensity"), FString::Printf(TEXT("%f"), LightData.Intensity));
    OutProperties.Add(TEXT("LightData_AttenuationRadius"), FString::Printf(TEXT("%f"), LightData.AttenuationRadius));
    OutProperties.Add(TEXT("LightData_Falloff"), FString::Printf(TEXT("%f"), LightData.Falloff));
    OutProperties.Add(TEXT("LightData_Position"), LightData.Position.ToString());
}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
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
    TempStr = InProperties.Find(TEXT("LightData_AttenuationRadius"));
    if (TempStr)
    {
        LightData.AttenuationRadius = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Falloff"));
    if (TempStr)
    {
        LightData.Falloff = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Position"));
    if (TempStr)
    {
        LightData.Position.InitFromString(*TempStr);
    }
    
}

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
