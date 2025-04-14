#pragma once
#include "LightComponentBase.h"
#include "Define.h"
class UBillboardComponent;

class ULightComponent : public ULightComponentBase
{
    DECLARE_CLASS(ULightComponent, ULightComponentBase)

public:
    ULightComponent();
    virtual ~ULightComponent() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) const override;
    void InitializeLight();

    template<typename T>
   T& GetLightData() { return *static_cast<T*>(GetLightDefinition()); }

protected:
    FBoundingBox AABB;
    
    /** Need to override in derived class */
    virtual void* GetLightDefinition() { return nullptr; }
public:
    FBoundingBox GetLocalBoundingBox() const {return AABB;}
};
