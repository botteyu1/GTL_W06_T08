#pragma once

#include "GameFramework/Actor.h"

class UExponentialHeightFogComponent;

class AHeightFogActor : public AActor
{
    DECLARE_CLASS(AHeightFogActor, AActor)

public:
    AHeightFogActor();

    UExponentialHeightFogComponent* HeightFogComponent;
};