#pragma once
#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
public:
    UDirectionalLightComponent();
    ~UDirectionalLightComponent() override = default;

    void TickComponent(float DeltaTime) override;
    
protected:
    void* GetLightDefinition() override;

private:
    FDirectionalLight LightData;
};
