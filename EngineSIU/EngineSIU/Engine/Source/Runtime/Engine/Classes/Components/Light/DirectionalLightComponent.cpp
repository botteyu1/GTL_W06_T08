#include "DirectionalLightComponent.h"

#include "UObject/Casts.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);
    LightData.Intensity = Intensity;
    LightData.Direction = this->GetForwardVector();
}

UObject* UDirectionalLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->LightData = LightData;

    return NewComponent;
}

void UDirectionalLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightData_Direction"), LightData.Direction.ToString());
    OutProperties.Add(TEXT("LightData_Intensity"), FString::Printf(TEXT("%f"), LightData.Intensity));
    OutProperties.Add(TEXT("LightData_Color"), LightData.Color.ToString());
    
}

void UDirectionalLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("LightData_Direction"));
    if (TempStr)
    {
        LightData.Direction.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Intensity"));
    if (TempStr)
    {
        LightData.Intensity = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightData_Color"));
    if (TempStr)
    {
        LightData.Color.InitFromString(*TempStr);
    }
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
