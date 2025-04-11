#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{

    DECLARE_CLASS(USpotLightComponent, ULightComponent)
public:
    USpotLightComponent();
    ~USpotLightComponent();
    FVector GetDirection();
    void SetDirection(const FVector& dir);

    float GetInnerConeAngle() const { return InnerConeAngle; }
    float GetOuterConeAngle() const { return OuterConeAngle; }

    void SetInnerConeAngle(float InAngle) { InnerConeAngle = InAngle; }
    void SetOuterConeAngle(float InAngle) { OuterConeAngle = InAngle; }

private:
    float InnerConeAngle;
    float OuterConeAngle;
};

