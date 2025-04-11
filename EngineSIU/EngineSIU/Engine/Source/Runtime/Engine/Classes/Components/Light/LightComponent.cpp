#include "LightComponent.h"
#include "Components/BillboardComponent.h"
#include "UObject/Casts.h"

ULightComponent::ULightComponent()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponent::~ULightComponent() = default;

UObject* ULightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->Intensity = this->Intensity;
    NewComponent->LightColor = this->LightColor;
    NewComponent->bVisible = this->bVisible;
    
    return NewComponent;
}

void ULightComponent::SetDiffuseColor(FLinearColor NewColor)
{
    // Light.DiffuseColor = FVector(NewColor.R, NewColor.G, NewColor.B);
    LightColor = FColor(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetSpecularColor(FLinearColor NewColor)
{
   // Light.SpecularColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetAttenuation(float Attenuation)
{
    // Light.Attenuation = Attenuation;
}

void ULightComponent::SetAttenuationRadius(float AttenuationRadius)
{
    // Light.AttRadius = AttenuationRadius;
}

void ULightComponent::SetIntensity(float Intensity)
{
    // Light.Intensity = Intensity;
}

void ULightComponent::SetFalloff(float fallOff)
{
    // Light.Falloff = fallOff;
}

FLinearColor ULightComponent::GetDiffuseColor()
{
    // return FLinearColor(Light.DiffuseColor.X, Light.DiffuseColor.Y, Light.DiffuseColor.Z, 1);
    return FLinearColor(LightColor.R, LightColor.G, LightColor.B, 1);
}

FLinearColor ULightComponent::GetSpecularColor()
{
    // return FLinearColor(Light.SpecularColor.X, Light.SpecularColor.Y, Light.SpecularColor.Z, 1);
    return FLinearColor::White;
}

float ULightComponent::GetAttenuation()
{
    // return Light.Attenuation;
    return 0.0f;
}

float ULightComponent::GetAttenuationRadius()
{
    // return Light.AttRadius;
    return 0.0f;
}

float ULightComponent::GetFalloff()
{
    // return Light.Falloff;
    return 0.0f;
}

FVector ULightComponent::GetColor() const
{
    return FVector(LightColor.R, LightColor.G, LightColor.B);
}

void ULightComponent::InitializeLight()
{  
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
}

void ULightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int ULightComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res = AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

