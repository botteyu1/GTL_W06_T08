#include "AmbientLightComponent.h"

#include "UObject/Casts.h"

UAmbientLightComponent::UAmbientLightComponent()
{
    LightData.Intensity = Intensity;
    LightData.Color = FVector(LightColor.R, LightColor.G, LightColor.B);
}

UObject* UAmbientLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->LightData = LightData;
    return NewComponent;
    
}

void UAmbientLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightData_Color"), LightData.Color.ToString());
    OutProperties.Add(TEXT("LightData_Intensity"), FString::Printf(TEXT("%f"), LightData.Intensity));
    
    
}

void UAmbientLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
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
}

void* UAmbientLightComponent::GetLightDefinition()
{
    return &LightData;
}
