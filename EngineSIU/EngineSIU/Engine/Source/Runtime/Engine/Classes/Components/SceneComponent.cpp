#include "Components/SceneComponent.h"

#include "GameFramework/Actor.h"
#include "Math/Rotator.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

USceneComponent::USceneComponent()
    : RelativeLocation(FVector(0.f, 0.f, 0.f))
    , RelativeRotation(FVector(0.f, 0.f, 0.f))
    , RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}

UObject* USceneComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->RelativeLocation = RelativeLocation;
    NewComponent->RelativeRotation = RelativeRotation;
    NewComponent->RelativeScale3D = RelativeScale3D;

    return NewComponent;
}

void USceneComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("RelativeLocation"), *RelativeLocation.ToString());
    OutProperties.Add(TEXT("RelativeRotation"), *RelativeRotation.ToString());
    OutProperties.Add(TEXT("RelativeScale3D"), *RelativeScale3D.ToString());
    USceneComponent* ParentComp = GetAttachParent();
    if (ParentComp != nullptr) {
        OutProperties.Add(TEXT("AttachParentID"), ParentComp->GetName());
    }
    else
    {
        OutProperties.Add(TEXT("AttachParentID"), "nullptr");
    }
    
}

void USceneComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("RelativeLocation"));
    if (TempStr)
    {
        RelativeLocation.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("RelativeRotation"));
    if (TempStr)
    {
        RelativeRotation.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("RelativeScale3D"));
    if (TempStr)
    {
        RelativeScale3D.InitFromString(*TempStr);
    }
}

void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

void USceneComponent::DestroyComponent(bool bPromoteChildren)
{
    TArray<USceneComponent*> ChildrenCopy = AttachChildren;
    for (auto& Child : ChildrenCopy)
    {
        if (Child == nullptr)
        {
            continue;
        }

        if (bPromoteChildren)
        {
            Child->DestroyComponent(bPromoteChildren);
        }
        else
        {
            AActor* Owner = GetOwner();
            if (AttachParent)
            {
                // 자식 컴포넌트들을 부모에 어태치
                Child->SetupAttachment(AttachParent);
            }
            else if (Owner != nullptr)
            {
                if (Owner->GetRootComponent())
                {
                    // 부모가 nullptr인 경우 Owner의 Root에라도 어태치
                    Child->SetupAttachment(Owner->GetRootComponent());
                }
                else
                {
                    // 루트 컴포넌트도 없는 경우, 아무거나 하나를 루트로 지정해줌
                    Owner->SetRootComponent(Child);       
                }
            }
        }
    }

    AttachChildren.Empty();

    if (AttachParent)
    {
        DetachFromComponent(AttachParent);
    }
    
    UActorComponent::DestroyComponent(bPromoteChildren);
}


int USceneComponent::CheckRayIntersection(FVector& InRayOrigin, FVector& InRayDirection, float& pfNearHitDistance) const
{
    int nIntersections = 0;
    return nIntersections;
}

FVector USceneComponent::GetForwardVector() const
{
    FVector4 Forward4 = FVector4(1.f, 0.f, 0.0f, 0.0f);
    Forward4 = FMatrix::TransformVector(Forward4, GetWorldMatrix());
    FVector Forward;
    Forward = FVector(Forward4.X, Forward4.Y, Forward4.Z);
    Forward = Forward.Normalize();

    return Forward;
}

FVector USceneComponent::GetRightVector() const
{
    FVector4 Right4 = FVector4(0.f, 0.f, 1.0f, 0.0f);
    Right4 = FMatrix::TransformVector(Right4, GetWorldMatrix());
    FVector Right;
    Right = FVector(Right4.X, Right4.Y, Right4.Z);
    Right = Right.Normalize();

    return Right;
}

FVector USceneComponent::GetUpVector() const
{
    FVector4 Up4 = FVector4(0.f, 0.f, 1.0f, 0.0f);
    Up4 = FMatrix::TransformVector(Up4, GetWorldMatrix());
    FVector Up;
    Up = FVector(Up4.X, Up4.Y, Up4.Z);
    Up = Up.Normalize();

    return Up;
}


void USceneComponent::AddLocation(FVector InAddValue)
{
	RelativeLocation = RelativeLocation + InAddValue;

}

void USceneComponent::AddRotation(FVector InAddValue)
{
	RelativeRotation = RelativeRotation + InAddValue;

}

void USceneComponent::AddScale(FVector InAddValue)
{
	RelativeScale3D = RelativeScale3D + InAddValue;

}

void USceneComponent::AttachToComponent(USceneComponent* InParent)
{
    // 기존 부모와 연결을 끊기
    if (AttachParent)
    {
        AttachParent->AttachChildren.Remove(this);
    }

    // InParent도 nullptr이면 부모를 nullptr로 설정
    if (InParent == nullptr)
    {
        AttachParent = nullptr;
        return;
    }


    // 새로운 부모 설정
    AttachParent = InParent;

    // 부모의 자식 리스트에 추가
    if (!InParent->AttachChildren.Contains(this))
    {
        InParent->AttachChildren.Add(this);
    }
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (
        InParent != AttachParent                                  // 설정하려는 Parent가 기존의 Parent와 다르거나
        && InParent != this                                       // InParent가 본인이 아니고
        && InParent != nullptr                                    // InParent가 유효한 포인터 이며
        && (
            AttachParent == nullptr                               // AttachParent도 유효하며
            || !AttachParent->AttachChildren.Contains(this)  // 한번이라도 SetupAttachment가 호출된적이 없는 경우
        ) 
    ) {
        AttachParent = InParent;

        // TODO: .AddUnique의 실행 위치를 RegisterComponent로 바꾸거나 해야할 듯
        InParent->AttachChildren.AddUnique(this);
    }
}

void USceneComponent::DetachFromComponent(USceneComponent* Target)
{
    // TODO: Detachment Rule 필요

    if (!Target || !Target->AttachChildren.Contains(this))
    {
        return;
    }

    Target->AttachChildren.Remove(this);
}

FVector USceneComponent::GetWorldLocation() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldMatrix().TransformPosition(RelativeLocation);
    }
    return RelativeLocation;
}

FRotator USceneComponent::GetWorldRotation() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldRotation().ToQuaternion() * RelativeRotation.ToQuaternion();
    }
    return RelativeRotation;
}

FVector USceneComponent::GetWorldScale3D() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldScale3D() * RelativeScale3D;
    }
    return RelativeScale3D;
}

FMatrix USceneComponent::GetScaleMatrix() const
{
    FMatrix ScaleMat = FMatrix::GetScaleMatrix(RelativeScale3D);
    if (AttachParent)
    {
        FMatrix ParentScaleMat = AttachParent->GetScaleMatrix();
        ScaleMat = ScaleMat * ParentScaleMat;
    }
    return ScaleMat;
}

FMatrix USceneComponent::GetRotationMatrix() const
{
    FMatrix RotationMat = FMatrix::GetRotationMatrix(RelativeRotation);
    if (AttachParent)
    {
        FMatrix ParentRotationMat = AttachParent->GetRotationMatrix();
        RotationMat = RotationMat * ParentRotationMat;
    }
    return RotationMat;
}

FMatrix USceneComponent::GetTranslationMatrix() const
{
    FMatrix TranslationMat = FMatrix::GetTranslationMatrix(RelativeLocation);
    if (AttachParent)
    {
        FMatrix ParentTranslationMat = AttachParent->GetTranslationMatrix();
        TranslationMat = TranslationMat * ParentTranslationMat;
    }
    return TranslationMat;
}

FMatrix USceneComponent::GetWorldMatrix() const
{
    FMatrix ScaleMat = FMatrix::GetScaleMatrix(RelativeScale3D);
    FMatrix RotationMat = FMatrix::GetRotationMatrix(RelativeRotation);
    FMatrix TranslationMat = FMatrix::GetTranslationMatrix(RelativeLocation);

    FMatrix RTMat = RotationMat * TranslationMat;
    if (AttachParent)
    {
        FMatrix ParentScaleMat = AttachParent->GetScaleMatrix();
        FMatrix ParentRotationMat = AttachParent->GetRotationMatrix();
        FMatrix ParentTranslationMat = AttachParent->GetTranslationMatrix();
        
        ScaleMat = ScaleMat * ParentScaleMat;
        FMatrix ParentRTMat = ParentRotationMat * ParentTranslationMat;
        RTMat = RTMat * ParentRTMat;
    }
    return ScaleMat * RTMat;
}
