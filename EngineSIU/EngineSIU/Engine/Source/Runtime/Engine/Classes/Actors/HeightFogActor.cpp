#include "HeightFogActor.h"

#include "Components/ExponentialHeightFogComponent.h"

AHeightFogActor::AHeightFogActor()
{
    HeightFogComponent = AddComponent<UExponentialHeightFogComponent>("UExponentialHeightFogComponent_0");

    
}
