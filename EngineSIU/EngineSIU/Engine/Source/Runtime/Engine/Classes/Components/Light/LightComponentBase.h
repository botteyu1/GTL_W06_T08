#pragma once

#include "Components/SceneComponent.h"
#include "Math/Color.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();

public:   
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
