#pragma once
#include "GameFramework/Actor.h"

class UPointLightComponent;
class UBillboardComponent;

class APointLight :public AActor
{
    DECLARE_CLASS(APointLight, AActor)
public:
    APointLight();
    ~APointLight() override;
protected:
  
    UPROPERTY
    (UPointLightComponent*, PointLightComponent, = nullptr);

   UPROPERTY
   (UBillboardComponent*, BillboardComponent, = nullptr);
};
