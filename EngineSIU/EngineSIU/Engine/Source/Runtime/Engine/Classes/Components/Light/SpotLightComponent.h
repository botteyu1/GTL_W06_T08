#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{
    DECLARE_CLASS(USpotLightComponent, ULightComponent)
    
public:
    USpotLightComponent();
    ~USpotLightComponent() override;

    
    virtual UObject* Duplicate(UObject* InOuter) override;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    void TickComponent(float DeltaTime) override;
    
    float GetAttenuationRadius() { return LightData.AttenuationRadius; }
    float GetInnerConeAngle() { return LightData.InnerConeAngle; }
    float GetOuterConeAngle() { return LightData.OuterConeAngle; }
    float GetFalloff() { return LightData.Falloff; }

    void SetAttenuationRadius(float AttenuationRadius) { LightData.AttenuationRadius = AttenuationRadius; }
    void SetInnerConeAngle(float InnerConeAngle);
    void SetOuterConeAngle(float OuterConeAngle);
    void SetFalloff(float Falloff) { LightData.Falloff = Falloff; }

protected:
    void* GetLightDefinition() override;
    
private:
    FSpotLight LightData;
};

