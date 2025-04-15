#include "PropertyEditorPanel.h"

#include "World/World.h"
#include "Actors/Player.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/FLoaderOBJ.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Engine.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/ProjectileMovementComponent.h"

#include "Engine/AssetManager.h"
#include "UObject/UObjectIterator.h"

void PropertyEditorPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }
    Padding = FSlateRect(5, 15, 1, -15.f);
    AnchorMin = FVector2D(0.8f, 0.3f);
    AnchorMax = FVector2D(1.f, 0.95f);
    
    FSlateRect PanelRect = FSlateRect(0 ,0, 0, 0);
    PanelRect.Left = Rect.GetWidth() * AnchorMin.X + Padding.Left;
    PanelRect.Top = Rect.GetHeight() * AnchorMin.Y + Padding.Top;

    PanelRect.Right = Rect.GetWidth() * AnchorMax.X - Padding.Right;
    PanelRect.Bottom = Rect.GetHeight() * AnchorMax.Y - Padding.Bottom;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelRect.Left, PanelRect.Top), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelRect.GetWidth(), PanelRect.GetHeight()), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("Detail", nullptr, PanelFlags);
    
    AEditorPlayer* player = Engine->GetEditorPlayer();
    AActor* SelectedActor = Engine->GetSelectedActor();
    USceneComponent* SelectedComponent = Engine->GetSelectedComponent();

    USceneComponent* TargetComponent = nullptr;
    
    if (SelectedComponent != nullptr)
    {
        TargetComponent = SelectedComponent;
    }
    else if (SelectedActor != nullptr)
    {
        TargetComponent = SelectedActor->GetRootComponent();
    }
    
    if (TargetComponent != nullptr)
    {
        RenderForSceneComponent(TargetComponent, player);
    }


    if (SelectedActor)
    {
        RenderForActor(SelectedActor, TargetComponent);
    }
    
    if (ULightComponent* LightComp = GetTargetComponent<ULightComponent>(SelectedActor, SelectedComponent))
    {
        RenderForLightComponent(LightComp);
    }

    if (UProjectileMovementComponent* ProjectileComp = GetTargetComponent<UProjectileMovementComponent>(SelectedActor, SelectedComponent))
    {
        RenderForProjectileMovementComponent(ProjectileComp);
    }

    if (UTextComponent* TextComp = GetTargetComponent<UTextComponent>(SelectedActor, SelectedComponent))
    {
        RenderForTextComponent(TextComp);
    }
    
    if (UStaticMeshComponent* StaticMeshComponent = GetTargetComponent<UStaticMeshComponent>(SelectedActor, SelectedComponent))
    {
        RenderForStaticMesh(StaticMeshComponent);
        RenderForMaterial(StaticMeshComponent);
    }

    if (UExponentialHeightFogComponent* FogComponent = GetTargetComponent<UExponentialHeightFogComponent>(SelectedActor, SelectedComponent))
    {
        RenderForExponentialHeightFogComponent(FogComponent);
    }
    
    ImGui::End();
}

void PropertyEditorPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const
{
    float mx = FMath::Max(r, FMath::Max(g, b));
    float mn = FMath::Min(r, FMath::Min(g, b));
    float delta = mx - mn;

    v = mx;

    if (mx == 0.0f) {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    else {
        s = delta / mx;
    }

    if (delta < 1e-6) {
        h = 0.0f;
    }
    else {
        if (r >= mx) {
            h = (g - b) / delta;
        }
        else if (g >= mx) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }
        h *= 60.0f;
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
}

void PropertyEditorPanel::HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const
{
    // h: 0~360, s:0~1, v:0~1
    float c = v * s;
    float hp = h / 60.0f;             // 0~6 구간
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
    else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
    else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
    else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
    else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
    else { r = c;  g = 0.0f; b = x; }

    r += m;  g += m;  b += m;
}

void PropertyEditorPanel::RenderForActor(AActor* SelectedActor, USceneComponent* TargetComponent) const
{
    if (ImGui::Button("Duplicate"))
    {
        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
        AActor* NewActor = Engine->ActiveWorld->DuplicateActor(SelectedActor);
        Engine->SelectActor(NewActor);
        Engine->DeselectComponent(Engine->GetSelectedComponent());
    }
    
    if (ImGui::TreeNodeEx("Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("Add");
        ImGui::SameLine();

        TArray<UClass*> CompClasses;
        GetChildOfClass(USceneComponent::StaticClass(), CompClasses);

        if (ImGui::BeginCombo("##AddComponent", "Components", ImGuiComboFlags_None))
        {
            for (UClass* Class : CompClasses)
            {
                if (ImGui::Selectable(GetData(Class->GetName()), false))
                {
                    USceneComponent* NewComp = Cast<USceneComponent>(SelectedActor->AddComponent(Class));
                    if (NewComp != nullptr && TargetComponent != nullptr)
                    {
                        NewComp->SetupAttachment(TargetComponent);
                    }
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
}

void PropertyEditorPanel::RenderForSceneComponent(USceneComponent* SceneComponent, AEditorPlayer* Player) const
{
    ImGui::SetItemDefaultFocus();

    if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        FVector Location = SceneComponent->GetRelativeLocation();
        FRotator Rotation = SceneComponent->GetRelativeRotation();
        FVector Scale = SceneComponent->GetRelativeScale3D();

        FImGuiWidget::DrawVec3Control("Location", Location, 0, 85);
        ImGui::Spacing();

        FImGuiWidget::DrawRot3Control("Rotation", Rotation, 0, 85);
        ImGui::Spacing();

        FImGuiWidget::DrawVec3Control("Scale", Scale, 0, 85);
        ImGui::Spacing();

        SceneComponent->SetRelativeLocation(Location);
        SceneComponent->SetRelativeRotation(Rotation);
        SceneComponent->SetRelativeScale3D(Scale);

        std::string CoordiButtonLabel;
        if (Player->GetCoordiMode() == CoordiMode::CDM_WORLD)
            CoordiButtonLabel = "World";
        else if (Player->GetCoordiMode() == CoordiMode::CDM_LOCAL)
            CoordiButtonLabel = "Local";

        if (ImGui::Button(CoordiButtonLabel.c_str(), ImVec2(ImGui::GetWindowContentRegionMax().x * 0.9f, 32)))
        {
            Player->AddCoordiMode();
        }
        ImGui::TreePop(); // 트리 닫기
    }

}

void PropertyEditorPanel::RenderForLightComponent(ULightComponent* LightComp) const
{
     if (ImGui::TreeNodeEx("Light Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
     {
         /*  DrawColorProperty("Ambient Color",
               [&]() { return lightObj->GetAmbientColor(); },
               [&](FVector4 c) { lightObj->SetAmbientColor(c); });
           */
         DrawColorProperty("Base Color",
             [&]() { return LightComp->GetLightColor(); },
             [&](FLinearColor c) { LightComp->SetLightColor(c); });
         
         float Intensity = LightComp->GetIntensity();
         if (ImGui::DragFloat("Intensity", &Intensity, 0.1f, 0.0f, 10000.0f, "%1.f"))
             LightComp->SetIntensity(Intensity);

         if (UPointLightComponent* PointLightComp = Cast<UPointLightComponent>(LightComp))
         {
             float Radius = PointLightComp->GetAttenuationRadius();
             if (ImGui::DragFloat("Radius", &Radius,0.1f, 0.0f, 10000.0f, "%1.f"))
             {
                 PointLightComp->SetAttenuationRadius(Radius);
             }

             float falloff = PointLightComp->GetFalloff();
             if (ImGui::DragFloat("Falloff", &falloff, 0.1f, 0.1f, 10.0f, "%.2f"))
             {
                 PointLightComp->SetFalloff(falloff);
             }
         }
         
         /*  
        float falloff = lightObj->GetFalloff();
        if (ImGui::SliderFloat("Falloff", &falloff, 0.1f, 10.0f, "%.2f")) {
            lightObj->SetFalloff(falloff);
        }

        */

        // TODO : For SpotLight
         if (USpotLightComponent* SpotComp = Cast<USpotLightComponent>(LightComp))
         {
             // AttenuationRadius
             float Radius = SpotComp->GetAttenuationRadius();
             if (ImGui::DragFloat("Radius", &Radius,0.1f, 0.0f, 10000.0f, "%1.f"))
             {
                 SpotComp->SetAttenuationRadius(Radius);
             }
             // InnerCone
             float InnerCone = SpotComp->GetInnerConeAngle();
             if (ImGui::DragFloat("InnerCone", &InnerCone, 0.1f, 0.0f, 80.0f, "%1.f"))
             {
                 SpotComp->SetInnerConeAngle(InnerCone);
             }
             // OuterCone
             float OuterCone = SpotComp->GetOuterConeAngle();
             if (ImGui::DragFloat("OuterCone", &OuterCone, 0.1f, 0.0f, 80.0f, "%1.f"))
             {
                 SpotComp->SetOuterConeAngle(OuterCone);
             }
             // Falloff
             float Falloff = SpotComp->GetFalloff();
             if (ImGui::DragFloat("Falloff", &Falloff, 0.1f, 0.1f, 10.0f, "%1.f"))
             {
                 SpotComp->SetFalloff(Falloff);
             }
         }
         
         ImGui::TreePop();
     }
}

void PropertyEditorPanel::RenderForProjectileMovementComponent(UProjectileMovementComponent* ProjectileComp) const
{
    if (ImGui::TreeNodeEx("Projectile Movement Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        float InitialSpeed = ProjectileComp->GetInitialSpeed();
        if (ImGui::InputFloat("InitialSpeed", &InitialSpeed, 0.f, 10000.0f, "%.1f"))
            ProjectileComp->SetInitialSpeed(InitialSpeed);

        float MaxSpeed = ProjectileComp->GetMaxSpeed();
        if (ImGui::InputFloat("MaxSpeed", &MaxSpeed, 0.f, 10000.0f, "%.1f"))
            ProjectileComp->SetMaxSpeed(MaxSpeed);

        float Gravity = ProjectileComp->GetGravity();
        if (ImGui::InputFloat("Gravity", &Gravity, 0.f, 10000.f, "%.1f"))
            ProjectileComp->SetGravity(Gravity); 
                
        float ProjectileLifetime = ProjectileComp->GetLifetime();
        if (ImGui::InputFloat("Lifetime", &ProjectileLifetime, 0.f, 10000.f, "%.1f"))
            ProjectileComp->SetLifetime(ProjectileLifetime);

        FVector currentVelocity = ProjectileComp->GetVelocity();

        float velocity[3] = { currentVelocity.X, currentVelocity.Y, currentVelocity.Z };

        if (ImGui::InputFloat3("Velocity", velocity, "%.1f")) {
            ProjectileComp->SetVelocity(FVector(velocity[0], velocity[1], velocity[2]));
        }
                
        ImGui::TreePop();
    }
}

void PropertyEditorPanel::RenderForTextComponent(UTextComponent* TextComp) const
{
    if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        if (TextComp) {
            TextComp->SetTexture(L"Assets/Texture/font.png");
            TextComp->SetRowColumnCount(106, 106);
            FWString wText = TextComp->GetText();
            int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string u8Text(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, u8Text.data(), len, nullptr, nullptr);

            static char buf[256];
            strcpy_s(buf, u8Text.c_str());

            ImGui::Text("Text: ", buf);
            ImGui::SameLine();
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            if (ImGui::InputText("##Text", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                TextComp->ClearText();
                int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                FWString newWText(wlen, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, buf, -1, newWText.data(), wlen);
                TextComp->SetText(newWText.c_str());
            }
            ImGui::PopItemFlag();
        }
        ImGui::TreePop();
    }
}

void PropertyEditorPanel::RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const
{
    if (ImGui::TreeNodeEx("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("StaticMesh");
        ImGui::SameLine();

        FString PreviewName;
        if (StaticMeshComp->GetStaticMesh())
        {
            PreviewName = StaticMeshComp->GetStaticMesh()->GetRenderData()->DisplayName;
        }
        else
        {
            PreviewName = TEXT("None");
        }
        
        const TMap<FName, FAssetInfo> Assets = UAssetManager::Get().GetAssetRegistry();

        if (ImGui::BeginCombo("##StaticMesh", GetData(PreviewName), ImGuiComboFlags_None))
        {
            if (ImGui::Selectable(TEXT("None"), false))
            {
                StaticMeshComp->SetStaticMesh(nullptr);
            }
            
            for (const auto& Asset : Assets)
            {
                if (ImGui::Selectable(GetData(Asset.Value.AssetName.ToString()), false))
                {
                    FString MeshName = Asset.Value.PackagePath.ToString() + "/" + Asset.Value.AssetName.ToString();
                    UStaticMesh* StaticMesh = FManagerOBJ::GetStaticMesh(MeshName.ToWideString());
                    if (StaticMesh)
                    {
                        StaticMeshComp->SetStaticMesh(StaticMesh);
                    }
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
}


void PropertyEditorPanel::RenderForMaterial(UStaticMeshComponent* StaticMeshComp)
{
    if (StaticMeshComp->GetStaticMesh() == nullptr)
    {
        return;
    }
    
    if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        for (uint32 i = 0; i < StaticMeshComp->GetNumMaterials(); ++i)
        {
            if (ImGui::Selectable(GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    std::cout << GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()) << std::endl;
                    SelectedMaterialIndex = i;
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }

        if (ImGui::Button("    +    ")) {
            IsCreateMaterial = true;
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("SubMeshes", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        auto subsets = StaticMeshComp->GetStaticMesh()->GetRenderData()->MaterialSubsets;
        for (uint32 i = 0; i < subsets.Num(); ++i)
        {
            std::string temp = "subset " + std::to_string(i);
            if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    StaticMeshComp->SetselectedSubMeshIndex(i);
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }
        std::string temp = "clear subset";
        if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
                StaticMeshComp->SetselectedSubMeshIndex(-1);
        }

        ImGui::TreePop();
    }

    if (SelectedMaterialIndex != -1)
    {
        RenderMaterialView(SelectedStaticMeshComp->GetMaterial(SelectedMaterialIndex));
    }
    if (IsCreateMaterial) {
        RenderCreateMaterialView();
    }
}

void PropertyEditorPanel::RenderMaterialView(UMaterial* Material)
{
    ImGui::SetNextWindowSize(ImVec2(380, 400), ImGuiCond_Once);
    ImGui::Begin("Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    FVector MatDiffuseColor = Material->GetMaterialInfo().Diffuse;
    FVector MatSpecularColor = Material->GetMaterialInfo().Specular;
    FVector MatAmbientColor = Material->GetMaterialInfo().Ambient;
    FVector MatEmissiveColor = Material->GetMaterialInfo().Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Material Name |");
    ImGui::SameLine();
    ImGui::Text(*Material->GetMaterialInfo().MaterialName);
    ImGui::Separator();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        Material->SetDiffuse(NewColor);
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        Material->SetSpecular(NewColor);
    }


    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        Material->SetAmbient(NewColor);
    }


    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        Material->SetEmissive(NewColor);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Choose Material");
    ImGui::Spacing();

    ImGui::Text("Material Slot Name |");
    ImGui::SameLine();
    ImGui::Text(GetData(SelectedStaticMeshComp->GetMaterialSlotNames()[SelectedMaterialIndex].ToString()));

    ImGui::Text("Override Material |");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    // 메테리얼 이름 목록을 const char* 배열로 변환
    std::vector<const char*> materialChars;
    for (const auto& material : FManagerOBJ::GetMaterials()) {
        materialChars.push_back(*material.Value->GetMaterialInfo().MaterialName);
    }

    //// 드롭다운 표시 (currentMaterialIndex가 범위를 벗어나지 않도록 확인)
    //if (currentMaterialIndex >= FManagerOBJ::GetMaterialNum())
    //    currentMaterialIndex = 0;

    if (ImGui::Combo("##MaterialDropdown", &CurMaterialIndex, materialChars.data(), FManagerOBJ::GetMaterialNum())) {
        UMaterial* material = FManagerOBJ::GetMaterial(materialChars[CurMaterialIndex]);
        SelectedStaticMeshComp->SetMaterial(SelectedMaterialIndex, material);
    }

    if (ImGui::Button("Close"))
    {
        SelectedMaterialIndex = -1;
        SelectedStaticMeshComp = nullptr;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderForExponentialHeightFogComponent(UExponentialHeightFogComponent* ExponentialHeightFogComp) const
{
    if (ImGui::TreeNodeEx("Exponential Height Fog", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        FLinearColor currColor = ExponentialHeightFogComp->GetFogColor();

        float r = currColor.R;
        float g = currColor.G;
        float b = currColor.B;
        float a = currColor.A;
        float h, s, v;
        float lightColor[4] = { r, g, b, a };

        // SpotLight Color
        if (ImGui::ColorPicker4("##SpotLight Color", lightColor, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float))
        {
            r = lightColor[0];
            g = lightColor[1];
            b = lightColor[2];
            a = lightColor[3];
            ExponentialHeightFogComp->SetFogColor(FLinearColor(r, g, b, a));
        }
        RGBToHSV(r, g, b, h, s, v);
        // RGB/HSV
        bool changedRGB = false;
        bool changedHSV = false;

        // RGB
        ImGui::PushItemWidth(50.0f);
        if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;
        ImGui::Spacing();

        // HSV
        if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
        ImGui::PopItemWidth();
        ImGui::Spacing();

        if (changedRGB && !changedHSV)
        {
            // RGB -> HSV
            RGBToHSV(r, g, b, h, s, v);
            ExponentialHeightFogComp->SetFogColor(FLinearColor(r, g, b, a));
        }
        else if (changedHSV && !changedRGB)
        {
            // HSV -> RGB
            HSVToRGB(h, s, v, r, g, b);
            ExponentialHeightFogComp->SetFogColor(FLinearColor(r, g, b, a));
        }

        float FogDensity = ExponentialHeightFogComp->GetFogDensity();
        if (ImGui::SliderFloat("Density", &FogDensity, 0.00f, 1.0f))
        {
            ExponentialHeightFogComp->SetFogDensity(FogDensity);
        }

        float FogMaxOpacity = ExponentialHeightFogComp->GetFogMaxOpacity();
        if (ImGui::SliderFloat("Max Opacity", &FogMaxOpacity, 0.00f, 1.0f))
        {
            ExponentialHeightFogComp->SetFogMaxOpacity(FogMaxOpacity);
        }

        float FogHeightFallOff = ExponentialHeightFogComp->GetFogHeightFalloff();
        if (ImGui::SliderFloat("Height Fall Off", &FogHeightFallOff, 0.001f, 0.15f))
        {
            ExponentialHeightFogComp->SetFogHeightFalloff(FogHeightFallOff);
        }

        float FogStartDistance = ExponentialHeightFogComp->GetStartDistance();
        if (ImGui::SliderFloat("Start Distance", &FogStartDistance, 0.00f, 50.0f))
        {
            ExponentialHeightFogComp->SetStartDistance(FogStartDistance);
        }

        ImGui::TreePop();
    }
}

void PropertyEditorPanel::RenderCreateMaterialView()
{
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
    ImGui::Begin("Create Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    ImGui::Text("New Name");
    ImGui::SameLine();
    static char materialName[256] = "New Material";
    // 기본 텍스트 입력 필드
    ImGui::SetNextItemWidth(128);
    if (ImGui::InputText("##NewName", materialName, IM_ARRAYSIZE(materialName))) {
        tempMaterialInfo.MaterialName = materialName;
    }

    FVector MatDiffuseColor = tempMaterialInfo.Diffuse;
    FVector MatSpecularColor = tempMaterialInfo.Specular;
    FVector MatAmbientColor = tempMaterialInfo.Ambient;
    FVector MatEmissiveColor = tempMaterialInfo.Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Set Property");
    ImGui::Indent();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        tempMaterialInfo.Diffuse = NewColor;
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        tempMaterialInfo.Specular = NewColor;
    }


    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        tempMaterialInfo.Ambient = NewColor;
    }


    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        tempMaterialInfo.Emissive = NewColor;
    }
    ImGui::Unindent();

    ImGui::NewLine();
    if (ImGui::Button("Create Material")) {
        FManagerOBJ::CreateMaterial(tempMaterialInfo);
    }

    ImGui::NewLine();
    if (ImGui::Button("Close"))
    {
        IsCreateMaterial = false;
    }

    ImGui::End();
}

void PropertyEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Rect.Left = clientRect.left;
    Rect.Right = clientRect.right;
    Rect.Top = clientRect.top;
    Rect.Bottom = clientRect.bottom;
}


template<typename T>
    requires std::derived_from<T, UActorComponent>
T* PropertyEditorPanel::GetTargetComponent(AActor* SelectedActor, USceneComponent* SelectedComponent)
{
    T* ResultComp = nullptr;
    if (SelectedComponent != nullptr)
    {
        ResultComp = Cast<T>(SelectedComponent);
    }
    else if (SelectedActor != nullptr)
    {
        ResultComp = SelectedActor->GetComponentByClass<T>();
    }
        
    return ResultComp;
}
