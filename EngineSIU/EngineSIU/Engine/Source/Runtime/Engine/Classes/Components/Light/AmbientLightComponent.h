#pragma once

#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponent
{
    DECLARE_CLASS(UAmbientLightComponent, ULightComponent)
    
public:
    // Ambient Light
    UAmbientLightComponent();
    ~UAmbientLightComponent() override = default;

protected:
    void* GetLightDefinition() override;
    
private:
    FAmbientLight LightData;
};
