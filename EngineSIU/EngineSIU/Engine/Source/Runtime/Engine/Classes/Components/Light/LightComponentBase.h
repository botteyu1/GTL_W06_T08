#pragma once

#include "Components/SceneComponent.h"
#include "Math/Color.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();

public:
    virtual float GetIntensity() const;
    
    FColor GetLightColor() const;
    
    bool IsVisible() const;
    
protected:
    /** Power of light */
    float Intensity;

    /** Color of light */
    FColor LightColor;

    /** Can be visible */
    bool bVisible;

};
