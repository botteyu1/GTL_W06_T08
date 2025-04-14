#include "LightActor.h"
#include "Components/Light/PointLightComponent.h"
APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>("PointLightComponent_0");

    RootComponent = PointLightComponent;
}

APointLight::~APointLight()
{
}
