#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{
    DECLARE_CLASS(USpotLightComponent, ULightComponent)
    
public:
    USpotLightComponent();
    ~USpotLightComponent() override;

    void TickComponent(float DeltaTime) override;
    
    float GetAttenuationRadius() { return LightData.AttenuationRadius; }
    float GetInnerConeAngle() { return LightData.InnerConeAngle; }
    float GetOuterConeAngle() { return LightData.OuterConeAngle; }
    float GetFalloff() { return LightData.Falloff; }

    void SetAttenuationRadius(float AttenuationRadius) { LightData.AttenuationRadius = AttenuationRadius; }
    void SetInnerConeAngle(float InnerConeAngle) { LightData.InnerConeAngle = InnerConeAngle; }
    void SetOuterConeAngle(float OuterConeAngle) { LightData.OuterConeAngle = OuterConeAngle; }
    void SetFalloff(float Falloff) { LightData.Falloff = Falloff; }

protected:
    void* GetLightDefinition() override;
    
private:
    FSpotLight LightData;
};

