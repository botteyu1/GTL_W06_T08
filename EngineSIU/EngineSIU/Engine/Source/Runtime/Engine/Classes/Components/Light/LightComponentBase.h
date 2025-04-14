#pragma once

#include "Components/SceneComponent.h"
#include "Math/Color.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();

public:
    
    virtual UObject* Duplicate(UObject* InOuter) override;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    float GetIntensity() const { return Intensity; }
    FLinearColor GetLightColor() const { return LightColor; }
    bool IsVisible() const { return bVisible; }

    void SetIntensity(float InIntesity) { Intensity = InIntesity; }
    void SetLightColor(FLinearColor InColor) { LightColor = InColor; }
    void SetVisible(bool bValue) { bVisible = bValue; }
    
protected:
    /** Power of light */
    float Intensity;

    /** Color of light */
    FLinearColor LightColor;

    /** Can be visible */
    bool bVisible = true;

};
