#pragma once
#include "GameFramework/Actor.h"

class UAmbientLightComponent;

class AAmbientLight : public AActor
{
    DECLARE_CLASS(AAmbientLight, AActor)
public:
    AAmbientLight();
    ~AAmbientLight() override = default;

protected:
    UPROPERTY(UAmbientLightComponent*, AmbientLightComponent, = nullptr)
};
