#include "Player.h"

#include "UnrealClient.h"
#include "World/World.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/Light/LightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "Math/MathUtility.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "Engine/EditorEngine.h"


void AEditorPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Input();
}

void AEditorPlayer::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (!bLeftMouseDown)
        {
            bLeftMouseDown = true;

            POINT mousePos;
            GetCursorPos(&mousePos);
            GetCursorPos(&m_LastMousePos);

            //uint32 UUID = FEngineLoop::GraphicDevice.GetPixelUUID(mousePos);
            // TArray<UObject*> objectArr = GetWorld()->GetObjectArr();
            //for ( const USceneComponent* obj : TObjectRange<USceneComponent>())
            //{
            //    if (obj->GetUUID() != UUID) continue;

            //    UE_LOG(LogLevel::Display, *obj->GetName());
            //}
            ScreenToClient(GEngineLoop.AppWnd, &mousePos);

            FVector pickPosition;

            std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
            ScreenToViewSpace(mousePos.x, mousePos.y, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), pickPosition);
            bool res = PickGizmo(pickPosition, ActiveViewport.get());
            if (res == false)
            {
                PickActor(pickPosition);
            }
        }
        else
        {
            PickedObjControl();
        }
    }
    else
    {
        if (bLeftMouseDown)
        {
            bLeftMouseDown = false;
            std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
            ActiveViewport->SetPickedGizmoComponent(nullptr);
        }
    }
}

void AEditorPlayer::ProcessGizmoIntersection(UStaticMeshComponent* iter, const FVector& pickPosition, FEditorViewportClient* InActiveViewport, bool& isPickedGizmo)
{
    int maxIntersect = 0;
    float minDistance = FLT_MAX;
    float Distance = 0.0f;
    int currentIntersectCount = 0;
    if (!iter) return;
    if (RayIntersectsObject(pickPosition, iter, Distance, currentIntersectCount))
    {
        if (Distance < minDistance)
        {
            minDistance = Distance;
            maxIntersect = currentIntersectCount;
            //GetWorld()->SetPickingGizmo(iter);
            InActiveViewport->SetPickedGizmoComponent(iter);
            isPickedGizmo = true;
        }
        else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
        {
            maxIntersect = currentIntersectCount;
            //GetWorld()->SetPickingGizmo(iter);
            InActiveViewport->SetPickedGizmoComponent(iter);
            isPickedGizmo = true;
        }
    }
}

bool AEditorPlayer::PickGizmo(FVector& pickPosition, FEditorViewportClient* InActiveViewport)
{
    bool isPickedGizmo = false;
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (Engine->GetSelectedActor())
    {
        if (cMode == CM_TRANSLATION)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetArrowArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
        else if (cMode == CM_ROTATION)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetDiscArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
        else if (cMode == CM_SCALE)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetScaleArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
    }
    return isPickedGizmo;
}

void AEditorPlayer::PickActor(const FVector& pickPosition)
{
    auto LevelEditor = GEngineLoop.GetLevelEditor();

    for (const auto& ViewportClient : LevelEditor->GetViewportClients())
    {
        if (ViewportClient->GetViewport()->GetFSlateRect().Contains(FVector2D(pickPosition.X, pickPosition.Y)))
        {
            if (!(ViewportClient->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives)))
            {
                return;
            }        
        }
    }
    
    USceneComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;

    // primitive가 아닌 uscenecomponent부터 선택 (아이콘으로 나와서 항상 위에 표시되기 때문)
    for (auto iter : TObjectRange<USceneComponent>())
    {
        if (!iter->IsA<UPrimitiveComponent>())
        {
            float Distance = 0.0f;
            int currentIntersectCount = 0;
            if (RayIntersectsObject(pickPosition, iter, Distance, currentIntersectCount))
            {
                if (Distance < minDistance)
                {
                    minDistance = Distance;
                    maxIntersect = currentIntersectCount;
                    Possible = iter;
                }
                else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                {
                    maxIntersect = currentIntersectCount;
                    Possible = iter;
                }
            }
        }
    }

    // 선택 안되었으면 primitive 확인
    if (!Possible)
    {
        for (auto iter : TObjectRange<UPrimitiveComponent>())
        {
            if (iter && !iter->IsA<UGizmoBaseComponent>())
            {
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                if (RayIntersectsObject(pickPosition, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        Possible = iter;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        Possible = iter;
                    }
                }
            }
        }
    }

    if (Possible)
    {
        Cast<UEditorEngine>(GEngine)->SelectActor(Possible->GetOwner());
        Cast<UEditorEngine>(GEngine)->SelectComponent(Possible);
    }
}

void AEditorPlayer::AddControlMode()
{
    cMode = static_cast<ControlMode>((cMode + 1) % CM_END);
}

void AEditorPlayer::AddCoordiMode()
{
    cdMode = static_cast<CoordiMode>((cdMode + 1) % CDM_END);
}

void AEditorPlayer::ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& rayOrigin)
{
    auto FocusedViewportClient = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
    D3D11_VIEWPORT viewport = FocusedViewportClient->GetD3DViewport();
    
    float viewportX = screenX - viewport.TopLeftX;
    float viewportY = screenY - viewport.TopLeftY;

    rayOrigin.X = ((2.0f * viewportX / viewport.Width) - 1) / projectionMatrix[0][0];
    rayOrigin.Y = -((2.0f * viewportY / viewport.Height) - 1) / projectionMatrix[1][1];
    if (FocusedViewportClient->IsOrtho())
    {
        rayOrigin.Z = 0.0f;  // 오쏘 모드에서는 unproject 시 near plane 위치를 기준
    }
    else
    {
        rayOrigin.Z = 1.0f;  // 퍼스펙티브 모드: near plane
    }
}

int AEditorPlayer::RayIntersectsObject(const FVector& pickPosition, USceneComponent* obj, float& hitDistance, int& intersectCount)
{
    auto LevelEditor = GEngineLoop.GetLevelEditor();

    for (const auto& ViewportClient : LevelEditor->GetViewportClients())
    {
        if (ViewportClient->GetViewport()->GetFSlateRect().Contains(FVector2D(pickPosition.X, pickPosition.Y)))
        {
            if (!(ViewportClient->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)))
            {
                return 0;
            }        
        }
    }
    
    auto FocusedViewportClient = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
    FMatrix WorldMatrix = obj->GetWorldMatrix();
	FMatrix ViewMatrix = FocusedViewportClient->GetViewMatrix();
    
    bool bIsOrtho = FocusedViewportClient->IsOrtho();
    

    if (bIsOrtho)
    {
        // 오쏘 모드: ScreenToViewSpace()에서 계산된 pickPosition이 클립/뷰 좌표라고 가정
        FMatrix inverseView = FMatrix::Inverse(ViewMatrix);
        // pickPosition을 월드 좌표로 변환
        FVector worldPickPos = inverseView.TransformPosition(pickPosition);  
        // 오쏘에서는 픽킹 원점은 unproject된 픽셀의 위치
        FVector rayOrigin = worldPickPos;
        // 레이 방향은 카메라의 정면 방향 (평행)
        FVector orthoRayDir = FocusedViewportClient->ViewTransformOrthographic.GetForwardVector().GetSafeNormal();

        // 객체의 로컬 좌표계로 변환
        FMatrix LocalMatrix = FMatrix::Inverse(WorldMatrix);
        FVector LocalRayOrigin = LocalMatrix.TransformPosition(rayOrigin);
        FVector LocalRayDir = (LocalMatrix.TransformPosition(rayOrigin + orthoRayDir) - LocalRayOrigin).GetSafeNormal();
        
        intersectCount = obj->CheckRayIntersection(LocalRayOrigin, LocalRayDir, hitDistance);
        return intersectCount;
    }
    else
    {
        FMatrix inverseMatrix = FMatrix::Inverse(WorldMatrix * ViewMatrix);
        FVector cameraOrigin = { 0,0,0 };
        FVector pickRayOrigin = inverseMatrix.TransformPosition(cameraOrigin);
        // 퍼스펙티브 모드의 기존 로직 사용
        FVector transformedPick = inverseMatrix.TransformPosition(pickPosition);
        FVector rayDirection = (transformedPick - pickRayOrigin).GetSafeNormal();
        
        intersectCount = obj->CheckRayIntersection(pickRayOrigin, rayDirection, hitDistance);

        if (intersectCount > 0)
        {
            FVector LocalHitPoint = pickRayOrigin + rayDirection * hitDistance;

            FVector WorldHitPoint = WorldMatrix.TransformPosition(LocalHitPoint);

            FVector WorldRayOrigin;
            FMatrix InverseView = FMatrix::Inverse(ViewMatrix);
            WorldRayOrigin = InverseView.TransformPosition(cameraOrigin);

            float WorldDistance = FVector::Distance(WorldRayOrigin, WorldHitPoint);

            hitDistance = WorldDistance;
        }
        return intersectCount;
    }
}

void AEditorPlayer::PickedObjControl()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    FEditorViewportClient* ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient().get();
    if (Engine && Engine->GetSelectedActor() && ActiveViewport->GetPickedGizmoComponent())
    {
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        int32 deltaX = currentMousePos.x - m_LastMousePos.x;
        int32 deltaY = currentMousePos.y - m_LastMousePos.y;

        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();
        AActor* SelectedActor = Engine->GetSelectedActor();

        USceneComponent* TargetComponent = nullptr;
    
        if (SelectedComponent != nullptr)
        {
            TargetComponent = SelectedComponent;
        }
        else if (SelectedActor != nullptr)
        {
            TargetComponent = SelectedActor->GetRootComponent();
        }
        
        UGizmoBaseComponent* Gizmo = static_cast<UGizmoBaseComponent*>(ActiveViewport->GetPickedGizmoComponent());
        switch (cMode)
        {
        case CM_TRANSLATION:
            ControlTranslation(TargetComponent, Gizmo, deltaX, deltaY);
            break;
        case CM_SCALE:
            ControlScale(TargetComponent, Gizmo, deltaX, deltaY);

            break;
        case CM_ROTATION:
            ControlRotation(TargetComponent, Gizmo, deltaX, deltaY);
            break;
        default:
            break;
        }
        m_LastMousePos = currentMousePos;
    }
}

void AEditorPlayer::ControlRotation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    const auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
    const FViewportCameraTransform* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->ViewTransformPerspective
                                                        : &ActiveViewport->ViewTransformOrthographic;

    FVector CameraForward = ViewTransform->GetForwardVector();
    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();

    FQuat currentRotation = pObj->GetWorldRotation().ToQuaternion();

    FQuat rotationDelta;

    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleX)
    {
        float rotationAmount = (CameraUp.Z >= 0 ? -1.0f : 1.0f) * deltaY * 0.01f;
        rotationAmount = rotationAmount + (CameraRight.X >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;

        rotationDelta = FQuat(FVector(1.0f, 0.0f, 0.0f), rotationAmount); // ���� X �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleY)
    {
        float rotationAmount = (CameraRight.X >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;
        rotationAmount = rotationAmount + (CameraUp.Z >= 0 ? 1.0f : -1.0f) * deltaY * 0.01f;

        rotationDelta = FQuat(FVector(0.0f, 1.0f, 0.0f), rotationAmount); // ���� Y �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleZ)
    {
        float rotationAmount = (CameraForward.X <= 0 ? -1.0f : 1.0f) * deltaX * 0.01f;
        rotationDelta = FQuat(FVector(0.0f, 0.0f, 1.0f), rotationAmount); // ���� Z �� ���� ȸ��
    }
    if (cdMode == CDM_LOCAL)
    {
        pObj->SetRelativeRotation(currentRotation * rotationDelta);
    }
    else if (cdMode == CDM_WORLD)
    {
        pObj->SetRelativeRotation(rotationDelta * currentRotation);
    }
}

void AEditorPlayer::ControlTranslation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float DeltaX = static_cast<float>(deltaX);
    float DeltaY = static_cast<float>(deltaY);
    const auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
    const FViewportCameraTransform* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->ViewTransformPerspective
                                                        : &ActiveViewport->ViewTransformOrthographic;

    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();
    
    FVector WorldMoveDirection = (CameraRight * DeltaX + CameraUp * -DeltaY) * 0.1f;
    
    if (cdMode == CDM_LOCAL)
    {
        if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowX)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetForwardVector());
            pObj->AddLocation(pObj->GetForwardVector() * moveAmount);
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowY)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetRightVector());
            pObj->AddLocation(pObj->GetRightVector() * moveAmount);
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetUpVector());
            pObj->AddLocation(pObj->GetUpVector() * moveAmount);
        }
    }
    else if (cdMode == CDM_WORLD)
    {
        // 월드 좌표계에서 카메라 방향을 고려한 이동
        if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowX)
        {
            // 카메라의 오른쪽 방향을 X축 이동에 사용
            FVector moveDir = CameraRight * DeltaX * 0.05f;
            pObj->AddLocation(FVector(moveDir.X, 0.0f, 0.0f));
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowY)
        {
            // 카메라의 오른쪽 방향을 Y축 이동에 사용
            FVector moveDir = CameraRight * DeltaX * 0.05f;
            pObj->AddLocation(FVector(0.0f, moveDir.Y, 0.0f));
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
        {
            // 카메라의 위쪽 방향을 Z축 이동에 사용
            FVector moveDir = CameraUp * -DeltaY * 0.05f;
            pObj->AddLocation(FVector(0.0f, 0.0f, moveDir.Z));
        }
    }
}

void AEditorPlayer::ControlScale(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float DeltaX = static_cast<float>(deltaX);
    float DeltaY = static_cast<float>(deltaY);
    const auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
    const FViewportCameraTransform* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->ViewTransformPerspective
                                                        : &ActiveViewport->ViewTransformOrthographic;
    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();
    
    // 월드 좌표계에서 카메라 방향을 고려한 이동
    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleX)
    {
        // 카메라의 오른쪽 방향을 X축 이동에 사용
        FVector moveDir = CameraRight * DeltaX * 0.05f;
        pObj->AddScale(FVector(moveDir.X, 0.0f, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleY)
    {
        // 카메라의 오른쪽 방향을 Y축 이동에 사용
        FVector moveDir = CameraRight * DeltaX * 0.05f;
        pObj->AddScale(FVector(0.0f, moveDir.Y, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
    {
        // 카메라의 위쪽 방향을 Z축 이동에 사용
        FVector moveDir = CameraUp * -DeltaY * 0.05f;
        pObj->AddScale(FVector(0.0f, 0.0f, moveDir.Z));
    }
}
