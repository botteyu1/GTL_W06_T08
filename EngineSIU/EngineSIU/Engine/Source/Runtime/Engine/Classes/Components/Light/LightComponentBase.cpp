#include "LightComponentBase.h"

#include "UObject/Casts.h"

ULightComponentBase::ULightComponentBase()
{
    Intensity = 1.0f;
    LightColor = FColor::White;
}

UObject* ULightComponentBase::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->Intensity = Intensity;
    NewComponent->LightColor = LightColor;
    NewComponent->bVisible = bVisible;
    

    return NewComponent;
}

void ULightComponentBase::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightColor"), LightColor.ToString());
    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), Intensity));
    OutProperties.Add(TEXT("bVisible"), bVisible ? TEXT("true") : TEXT("false"));
    
}

void ULightComponentBase::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    
    const FString* TempStr = nullptr;
    TempStr= InProperties.Find(TEXT("LightColor"));
    if (TempStr)
    {
        LightColor = FLinearColor(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        Intensity = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("bVisible"));
    if (TempStr)
    {
        bVisible = FCString::ToBool(**TempStr);
    }
}

