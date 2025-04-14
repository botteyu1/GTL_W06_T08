#pragma once
#include "LightComponent.h"

class UPointLightComponent :public ULightComponent
{

    DECLARE_CLASS(UPointLightComponent, ULightComponent)
public:

    
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    void TickComponent(float DeltaTime) override;

    virtual void AddScale(FVector InAddValue) override;


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


