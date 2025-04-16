#include "WorldControlEditorPanel.h"

#include <random>
#include "ImGuiManager.h"
#include "Actors/AmbientLightActor.h"
#include "Actors/Cube.h"
#include "Actors/DirectionalLightActor.h"
#include "Actors/FireballActor.h"
#include "Actors/HeightFogActor.h"
#include "Actors/LightActor.h"
#include "Actors/SpotLightActor.h"
#include "Components/Light/PointLightComponent.h"

#include "Components/ParticleSubUVComponent.h"
#include "Components/SphereComp.h"
#include "Components/TextComponent.h"
#include "Engine/ConfigManager.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/FLoaderOBJ.h"

#include "GameFramework/Actor.h"
#include "LevelEditor/SLevelEditor.h"
#include "Renderer/StaticMeshRenderPass.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/ObjectTypes.h"
#include "World/World.h"

void WorldControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    Padding = FSlateRect(1, 1, 0, 0.f);
    AnchorMin = FVector2D(0.f, 0.f);
    AnchorMax = FVector2D(0.8f, 0.f);
    
    FSlateRect PanelRect = FSlateRect(0 ,0, 0, 0);
    PanelRect.Left = Rect.GetWidth() * AnchorMin.X + Padding.Left;
    PanelRect.Top = Rect.GetHeight() * AnchorMin.Y + Padding.Top;

    PanelRect.Right = Rect.GetWidth() * AnchorMax.X - Padding.Right;
    PanelRect.Bottom = Rect.GetHeight() * AnchorMax.Y - Padding.Bottom;

    PanelRect.SetHeight(45);

    ImVec2 MinSize(300, 50);
    ImVec2 MaxSize(FLT_MAX, 50);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelRect.Left, PanelRect.Top), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelRect.GetWidth(), PanelRect.GetHeight()), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    /* Render Start */
    ImGui::Begin("World Control Panel", nullptr, PanelFlags);

    CreateMenuButton(IconSize, IconFont);

    ImGui::SameLine();

    CreateSettingButton(IconSize, IconFont);
    
    ImGui::SameLine();

    CreateAddPrimitiveButton(IconSize, IconFont);

    ImGui::SameLine();
    
    /* Get Window Content Region */
    float ContentWidth = ImGui::GetWindowContentRegionMax().x;

    //ImGui::SameLine();
    ImGui::PushFont(IconFont);

    CreatePIEButton(IconSize, IconFont);

    ImGui::SameLine();

    /* Move Cursor X Position */
    ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));

    ImGui::PopFont();

    ImGui::End();
}

void WorldControlEditorPanel::CreateAddPrimitiveButton(ImVec2 ButtonSize, ImFont* IconFont) const
{    
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c8", ButtonSize))
    {
        ImGui::OpenPopup("PrimitiveControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("PrimitiveControl"))
    {
        struct Primitive {
            const char* label;
            int obj;
        };

        static const Primitive primitives[] = {
            { .label= "Cube",      .obj= OBJ_CUBE },
            { .label= "Sphere",    .obj= OBJ_SPHERE },
            { .label= "PointLight", .obj= OBJ_PointLight },
            {.label = "SpotLight", .obj= OBJ_SpotLight },
            {.label = "DirectionalLight", .obj= OBJ_DirectionalLight },
            {.label = "AmbientLight", .obj= OBJ_AmbientLight },
            { .label= "Particle",  .obj= OBJ_PARTICLE },
            { .label= "Text",      .obj= OBJ_Text },
            { .label= "Fireball",  .obj = OBJ_Fireball},
            { .label= "Fog",       .obj= OBJ_Fog }
        };

        for (const auto& primitive : primitives)
        {
            if (ImGui::Selectable(primitive.label))
            {
                // GEngineLoop.GetWorld()->SpawnObject(static_cast<OBJECTS>(primitive.obj));
                UWorld* World = GEngine->ActiveWorld;
                AActor* SpawnedActor = nullptr;
                switch (static_cast<OBJECTS>(primitive.obj))
                {
                case OBJ_SPHERE:
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_SPHERE"));
                    USphereComp* SphereComp = SpawnedActor->AddComponent<USphereComp>();
                    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));
                    break;
                }
                case OBJ_CUBE:
                {
                    // TODO: 다른 부분들 전부 Actor만 소환하도록 하고, Component 생성은 Actor가 자체적으로 하도록 변경.
                    ACube* CubeActor = World->SpawnActor<ACube>();
                    CubeActor->SetActorLabel(TEXT("OBJ_CUBE"));
                    break;
                }
                case OBJ_PointLight:
                {
                    APointLight* LightActor = World->SpawnActor<APointLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_PointLight"));
                    break;
                }
                case OBJ_SpotLight:
                {
                    ASpotLight* LightActor = World->SpawnActor<ASpotLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_SpotLight"));
                    break;
                }
                case OBJ_DirectionalLight:
                {
                    ADirectionalLight* LightActor = World->SpawnActor<ADirectionalLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_DirectionalLight"));
                    break;
                        
                }
                case OBJ_AmbientLight:
                {
                    AAmbientLight* LightActor = World->SpawnActor<AAmbientLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_AmbientLight"));
                    break;
                }
                case OBJ_PARTICLE:
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_PARTICLE"));
                    UParticleSubUVComponent* ParticleComponent = SpawnedActor->AddComponent<UParticleSubUVComponent>();
                    ParticleComponent->SetTexture(L"Assets/Texture/T_Explosion_SubUV.png");
                    ParticleComponent->SetRowColumnCount(6, 6);
                    ParticleComponent->SetRelativeScale3D(FVector(10.0f, 10.0f, 1.0f));
                    ParticleComponent->Activate();
                    SpawnedActor->SetActorTickInEditor(true);
                    break;
                }
                case OBJ_Text:
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_Text"));
                    UTextComponent* TextComponent = SpawnedActor->AddComponent<UTextComponent>();
                    TextComponent->SetTexture(L"Assets/Texture/font.png");
                    TextComponent->SetRowColumnCount(106, 106);
                    TextComponent->SetText(L"안녕하세요 Jungle 1");
                    
                    break;
                }
                case OBJ_Fireball:
                {
                    SpawnedActor = World->SpawnActor<AFireballActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_Fireball"));

                    break;
                }
                case OBJ_Fog:
                {
                    SpawnedActor = World->SpawnActor<AHeightFogActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_HeightFog"));
                    break;
                }
                case OBJ_TRIANGLE:
                case OBJ_CAMERA:
                case OBJ_PLAYER:
                case OBJ_END:
                    break;
                }

                if (SpawnedActor)
                {
                    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
                    Engine->DeselectComponent(Engine->GetSelectedComponent());
                    Engine->SelectActor(SpawnedActor);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void WorldControlEditorPanel::CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        bOpenMenu = !bOpenMenu;
    }
    ImGui::PopFont();

    if (bOpenMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 55), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(135, 170), ImGuiCond_Always);

        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (ImGui::MenuItem("New Level"))
        {
            if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
            {
                EditorEngine->NewLevel();
            }
            bOpenMenu = !bOpenMenu;
        }

        if (ImGui::MenuItem("Load Level"))
        {
            char const* lFilterPatterns[1] = { "*.scene" };
            const char* FileName = tinyfd_openFileDialog("Open Scene File", "", 1, lFilterPatterns, "Scene(.scene) file", 0);

            if (FileName == nullptr)
            {
                tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                bOpenMenu = false;
                ImGui::End();
                return;
            }
            if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
            {
                EditorEngine->NewLevel();
                EditorEngine->LoadLevel(FileName);
            }

            bOpenMenu = false;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Level"))
        {
            char const* lFilterPatterns[1] = { "*.scene" };
            const char* FileName = tinyfd_saveFileDialog("Save Scene File", "", 1, lFilterPatterns, "Scene(.scene) file");

            if (FileName == nullptr)
            {
                ImGui::End();
                return;
            }
            if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
            {
                EditorEngine->SaveLevel(FileName);
            }


            tinyfd_messageBox("알림", "저장되었습니다.", "ok", "info", 1);
            bOpenMenu = false;
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("Wavefront (.obj)"))
            {
                char const* lFilterPatterns[1] = { "*.obj" };
                const char* FileName = tinyfd_openFileDialog("Open OBJ File", "", 1, lFilterPatterns, "Wavefront(.obj) file", 0);

                if (FileName != nullptr)
                {
                    std::cout << FileName << std::endl;

                    if (FManagerOBJ::CreateStaticMesh(FileName) == nullptr)
                    {
                        tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                    }
                }
                bOpenMenu = false;
            }
            
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            ImGui::OpenPopup("Quit Program");
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Quit Program", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::Text("Are you sure you want to quit?");
            ImGui::Separator();

            float ContentWidth = ImGui::GetWindowContentRegionMax().x;

            /* Move Cursor X Position */
            ImGui::SetCursorPosX(ContentWidth - (160.f + 10.0f));

            if (ImGui::Button("OK", ImVec2(80, 0))) { PostQuitMessage(0); }

            ImGui::SameLine();

            ImGui::SetItemDefaultFocus();
            ImGui::PushID("CancelButtonWithQuitWindow");
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            if (ImGui::Button("Cancel", ImVec2(80, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::EndPopup();
        }

        ImGui::End();
    }
}

void WorldControlEditorPanel::CreateSettingButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9db", ButtonSize))
    {
        ImGui::OpenPopup("SettingControl##");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SettingControl##"))
    {
        if (ImGui::BeginMenu("Style Theme"))
        {
            if (ImGui::MenuItem("Light"))
            {
                GEngineLoop.GetUIManager()->PreferenceStyle(Light);
                ConfigManager::SetConfigValue("Preference", "Theme", FString(std::to_string((uint8)EEditorLayoutTheme::Light)), FName("settings.ini"));
            }

            if (ImGui::MenuItem("Dark"))
            {
                GEngineLoop.GetUIManager()->PreferenceStyle(Dark);
                ConfigManager::SetConfigValue("Preference", "Theme", FString(std::to_string((uint8)EEditorLayoutTheme::Dark)), FName("settings.ini"));
            }
            
            ImGui::EndMenu();
        }
    
        ImGui::Separator();
        
        static bool IsUber;
        IsUber = GEngineLoop.Renderer.StaticMeshRenderPass->IsUber();
        if (ImGui::Checkbox("UberLit.hlsl(O)", &IsUber))
        {
            GEngineLoop.Renderer.StaticMeshRenderPass->SetUberShader(IsUber);
        }

        if (ImGui::Button("Recompile(Shift + P)"))
        {

            EViewModeIndex mode = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient()->GetViewMode();
            switch (mode)
            {
            case VMI_LitGouraud:
                GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(1, 0, 0, true);
                break;
            case VMI_LitLambert:
                GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(0, 1, 0, true);
                break;
            case VMI_LitBlinnPhong:
                GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(0, 0, 1, true);
                break;
            default:
                GEngineLoop.Renderer.StaticMeshRenderPass->UpdateShaders(1, 0, 0, true);
                break;
            }
        }
        static bool IsAutoUpdate = false;
        if (ImGui::Checkbox("AutoRecompile", &IsAutoUpdate))
        {
            GEngineLoop.Renderer.StaticMeshRenderPass->SetAutoUpdate(IsAutoUpdate);
        }
        ImGui::EndPopup();
    }
}

void WorldControlEditorPanel::CreatePIEButton(ImVec2 ButtonSize, ImFont* IconFont) const
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return;

    ImVec2 WindowSize = ImGui::GetIO().DisplaySize;

    float CenterX = (WindowSize.x * 0.8f) / 2;

    ImGui::SetCursorScreenPos(ImVec2(CenterX - (ButtonSize.x * 0.5f) - 5.0f, 10.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.0f);
    if (ImGui::Button("\ue9a8", ButtonSize)) // Play
    {
        UE_LOG(LogLevel::Display, TEXT("PIE Button Clicked"));
        Engine->StartPIE();
    }

    ImGui::SetCursorScreenPos(ImVec2(CenterX + (ButtonSize.x * 0.5f) + 5.0f, 10.0f));
    if (ImGui::Button("\ue9e4", ButtonSize)) // Stop
    {
        UE_LOG(LogLevel::Display, TEXT("Stop Button Clicked"));
        Engine->EndPIE();
    }
    ImGui::PopStyleVar();
    
}

void WorldControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Rect.Left = clientRect.left;
    Rect.Right = clientRect.right;
    Rect.Top = clientRect.top;
    Rect.Bottom = clientRect.bottom;
}
