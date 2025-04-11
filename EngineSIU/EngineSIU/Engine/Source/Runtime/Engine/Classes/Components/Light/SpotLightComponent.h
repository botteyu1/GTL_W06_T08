#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{
    DECLARE_CLASS(USpotLightComponent, ULightComponent)
    
public:
    USpotLightComponent();
    ~USpotLightComponent() override;

    void TickComponent(float DeltaTime) override;
    
    FVector GetDirection() const;
    void SetDirection(const FVector& dir);

protected:
    void* GetLightDefinition() override;
    
private:
    FSpotLight LightData;
};

