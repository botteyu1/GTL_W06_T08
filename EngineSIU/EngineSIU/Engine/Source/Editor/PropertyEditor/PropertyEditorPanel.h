#pragma once

#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class UExponentialHeightFogComponent;
class ULightComponent;
class UTextComponent;
class UProjectileMovementComponent;
class ULightComponentBase;
class AEditorPlayer;
class USceneComponent;
class UStaticMeshComponent;

// 헬퍼 함수 예시
template<typename Getter, typename Setter>
void DrawColorProperty(const char* label, Getter get, Setter set)
{
    ImGui::PushItemWidth(200.0f);
    FLinearColor curr = get();
    float col[4] = { curr.R, curr.G, curr.B, curr.A };

    if (ImGui::ColorEdit4(label, col,
        ImGuiColorEditFlags_DisplayRGB |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoInputs |
        ImGuiColorEditFlags_Float))
    {
        set(FLinearColor(col[0], col[1], col[2], col[3]));
    }
    ImGui::PopItemWidth();
}


class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const;
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const;

    void RenderForActor(AActor* SelectedActor, USceneComponent* TargetComponent) const;
    void RenderForSceneComponent(USceneComponent* SceneComponent, AEditorPlayer* Player) const;
    void RenderForLightComponent(ULightComponent* LightComp) const;
    void RenderForProjectileMovementComponent(UProjectileMovementComponent* ProjectileComp) const;
    void RenderForTextComponent(UTextComponent* TextComp) const;
    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const;
    
    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderForExponentialHeightFogComponent(UExponentialHeightFogComponent* ExponentialHeightFogComp) const;

    
    void RenderCreateMaterialView();
    
    template<typename T>
        requires std::derived_from<T, UActorComponent>
    T* GetTargetComponent(AActor* SelectedActor, USceneComponent* SelectedComponent);
private:
    /* Material Property */
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial;
};
