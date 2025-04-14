#include "SpotLightActor.h"
#include "Components/Light/SpotLightComponent.h"

ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>("USpotLightComponent_0");
    RootComponent = SpotLightComponent;
}
