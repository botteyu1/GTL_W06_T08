#pragma once
#include "LightComponent.h"

class UPointLightComponent :public ULightComponent
{

    DECLARE_CLASS(UPointLightComponent, ULightComponent)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    void TickComponent(float DeltaTime) override;
    
protected:
    void* GetLightDefinition() override;
    
private:
    FPointLight LightData;
};


