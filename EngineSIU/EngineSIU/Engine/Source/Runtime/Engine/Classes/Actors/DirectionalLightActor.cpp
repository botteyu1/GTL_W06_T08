#include "DirectionalLightActor.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/BillboardComponent.h"

ADirectionalLight::ADirectionalLight()
{
    DirectionalLightComponent = AddComponent<UDirectionalLightComponent>();
    RootComponent = DirectionalLightComponent;
}
