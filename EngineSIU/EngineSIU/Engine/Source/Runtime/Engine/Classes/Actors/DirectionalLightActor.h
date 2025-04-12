#pragma once
#include "GameFramework/Actor.h"

class UDirectionalLightComponent;

class ADirectionalLight : public AActor
{
    DECLARE_CLASS(ADirectionalLight, AActor)
public:
    ADirectionalLight();
    ~ADirectionalLight() override = default;

protected:
    UPROPERTY(UDirectionalLightComponent*, DirectionalLightComponent, = nullptr)
};
