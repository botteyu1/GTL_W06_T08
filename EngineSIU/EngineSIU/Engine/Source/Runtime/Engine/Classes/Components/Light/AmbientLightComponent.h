#pragma once

#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponent
{
    DECLARE_CLASS(UAmbientLightComponent, ULightComponent)
    
public:
    // Ambient Light
    UAmbientLightComponent();
    ~UAmbientLightComponent() override = default;

    
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

protected:
    void* GetLightDefinition() override;
    
private:
    FAmbientLight LightData;
};
