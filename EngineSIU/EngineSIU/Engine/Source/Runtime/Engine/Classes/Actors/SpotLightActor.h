#pragma once
#include "GameFramework/Actor.h"

class UBillboardComponent;
class USpotLightComponent;

class ASpotLight : public AActor
{
    DECLARE_CLASS(ASpotLight, AActor)
public:
    ASpotLight();
    ~ASpotLight() override = default;

protected:
    UPROPERTY(USpotLightComponent*, SpotLightComponent, = nullptr)
    UPROPERTY(UBillboardComponent*, BillboardComponent, = nullptr)
};
