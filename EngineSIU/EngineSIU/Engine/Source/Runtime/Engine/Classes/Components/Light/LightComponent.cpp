#include "LightComponent.h"
#include "Components/BillboardComponent.h"
#include "UObject/Casts.h"
#include "Runtime/Renderer/EditorRenderPass.h"

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

    NewComponent->AABB = AABB;
    
    return NewComponent;
}

void ULightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("AABB_min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_max"), AABB.max.ToString());
}

void ULightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* AABBminStr = InProperties.Find(TEXT("AABB_min"));
    if (AABBminStr) AABB.min.InitFromString(*AABBminStr);
    const FString* AABBmaxStr = InProperties.Find(TEXT("AABB_max"));
    if (AABBmaxStr) AABB.max.InitFromString(*AABBmaxStr);
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


int ULightComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) const const
{
    FVector center = { 0,0,0 };
    // FEditorRenderer::RenderIcons에서도 수정 필요
    float radius = FEditorRenderPass::IconScale;

    FVector L = rayOrigin - center;

    float a = rayDirection.Dot(rayDirection);
    float b = 2.f * rayDirection.Dot(L);
    float c = L.Dot(L) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0.0f)
        return 0; // 교차 없음

    // 근 접점 t 계산 (두 개의 해 중 더 작은 값)
    float sqrtDiscriminant = sqrtf(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
    float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

    float t = (t1 >= 0.0f) ? t1 : ((t2 >= 0.0f) ? t2 : -1.0f);

    if (t >= 0.0f)
    {
        pfNearHitDistance = t;
        return 1;
    }

    return 0;
}
