#include "LightActor.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/BillboardComponent.h"
APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>();

    RootComponent = PointLightComponent;
}

APointLight::~APointLight()
{
}
