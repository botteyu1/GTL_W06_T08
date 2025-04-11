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

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void InitializeLight();
    
    void SetDiffuseColor(FLinearColor NewColor);
    void SetSpecularColor(FLinearColor NewColor);
    void SetAttenuation(float Attenuation);
    void SetAttenuationRadius(float AttenuationRadius);
    void SetIntensity(float Intensity);
    void SetFalloff(float fallOff);

    FLinearColor GetDiffuseColor();
    FLinearColor GetSpecularColor();
    float GetAttenuation();
    float GetAttenuationRadius();
    float GetFalloff();

    FVector GetColor() const;
    void SetColor(FLinearColor NewColor);

    template<typename T>
    T& GetLightData() { return *static_cast<T*>(GetLightDefinition()); }

protected:
    FBoundingBox AABB;
    
    /** Need to override in derived class */
    virtual void* GetLightDefinition() { return nullptr; }
public:
    FBoundingBox GetLocalBoundingBox() const {return AABB;}
};
