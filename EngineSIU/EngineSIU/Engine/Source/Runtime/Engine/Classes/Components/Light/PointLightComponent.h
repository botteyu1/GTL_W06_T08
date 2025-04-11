#pragma once
#include "LightComponent.h"

class UPointLightComponent :public ULightComponent
{

    DECLARE_CLASS(UPointLightComponent, ULightComponent)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    void TickComponent(float DeltaTime) override;
    
    // position은 scenecomponent 변수 가져감
    float GetAttenuationRadius() const { return LightData.AttenuationRadius; }
    float GetFalloff() const { return LightData.Falloff; }

    void SetAttenuationRadius(float InRadius) { LightData.AttenuationRadius = InRadius; }
    void SetFalloff(float InFalloff) { LightData.Falloff = InFalloff; }

protected:
    void* GetLightDefinition() override;
    
private:
    FPointLight LightData;
};


