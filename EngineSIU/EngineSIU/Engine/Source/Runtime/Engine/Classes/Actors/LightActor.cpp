#include "LightActor.h"
#include "Components/Light/PointLightComponent.h"
APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>();

    RootComponent = PointLightComponent;
}

APointLight::~APointLight()
{
}
