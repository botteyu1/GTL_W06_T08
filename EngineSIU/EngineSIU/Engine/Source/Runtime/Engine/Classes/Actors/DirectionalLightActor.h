#pragma once
#include "GameFramework/Actor.h"

class UBillboardComponent;
class UDirectionalLightComponent;

class ADirectionalLight : public AActor
{
    DECLARE_CLASS(ADirectionalLight, AActor)
public:
    ADirectionalLight();
    ~ADirectionalLight() override = default;

protected:
    UPROPERTY(UDirectionalLightComponent*, DirectionalLightComponent, = nullptr)
    UPROPERTY(UBillboardComponent*, BillboardComponent, = nullptr)
};
