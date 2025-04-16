#include "ViewportControlEditorPanel.h"

#include "Editor/UnrealEd/EditorViewportClient.h"

#include "Engine/EditorEngine.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/Classes/Actors/Player.h"
#include "Engine/Classes/Actors/LightActor.h"
#include "Engine/Classes/Components/TextComponent.h"
#include "LevelEditor/SLevelEditor.h"

void ViewportControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    Padding = FSlateRect(1, 50, 0, 0.f);
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
    ImGui::Begin("Viewport Control Panel", nullptr, PanelFlags);

    CreateFlagButton();

    ImGui::SameLine();

    CreateModifyButton(IconSize, IconFont);

    ImGui::SameLine();

    CreateSRTButton(IconSize, IconFont);

    ImGui::End();
}

void ViewportControlEditorPanel::CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c4", ButtonSize)) // Slider
    {
        ImGui::OpenPopup("SliderControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SliderControl"))
    {
        auto ActiveViewportClient = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();
        ImGui::Text("Grid Scale");
        float GridScale = ActiveViewportClient->GetGridSize();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Grid Scale", &GridScale, 0.1f, 1.0f, 20.0f, "%.1f"))
        {
            ActiveViewportClient->SetGridSize(GridScale);
        }
        ImGui::Separator();

        ImGui::Text("Camera FOV");
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Fov", &ActiveViewportClient->ViewFOV, 0.1f, 30.0f, 120.0f, "%.1f"))
        {
            //GEngineLoop.GetWorld()->GetCamera()->SetFOV(FOV);

        }
        ImGui::Spacing();

        ImGui::Text("Camera Speed");
        float CameraSpeed = ActiveViewportClient->GetCameraSpeedScalar();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##CamSpeed", &CameraSpeed, 0.1f, 0.198f, 192.0f, "%.1f"))
        {
            ActiveViewportClient->SetCameraSpeedScalar(CameraSpeed);
        }

        ImGui::EndPopup();
    }
}

void ViewportControlEditorPanel::CreateFlagButton() const
{
    auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetFocusedViewportClient();

    const char* ViewTypeNames[] = { "Perspective", "Top", "Bottom", "Left", "Right", "Front", "Back" };
    ELevelViewportType ActiveViewType = ActiveViewport->GetViewportType();
    FString TextViewType = ViewTypeNames[ActiveViewType];

    if (ImGui::Button(GetData(TextViewType), ImVec2(120, 32)))
    {
        // toggleViewState = !toggleViewState;
        ImGui::OpenPopup("ViewControl");
    }

    if (ImGui::BeginPopup("ViewControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewTypeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewportType() == i);
            if (ImGui::Selectable(ViewTypeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewportType((ELevelViewportType)i);
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    const char* ViewModeNames[] = { "Lit_Gouraud", "Lit_Lambert", "Lit_BlinnPhong", "Unlit", "Wireframe", "SceneDepth", "WorldNormal", "WorldPos", "Attenuation"};
    int rawViewMode = (int)ActiveViewport->GetViewMode();
    int safeIndex = (rawViewMode >= 0) ? (rawViewMode % 9) : 0;
    FString ViewModeControl = ViewModeNames[safeIndex];

    ImVec2 ViewModeTextSize = ImGui::CalcTextSize(GetData(ViewModeControl));

    if (ImGui::Button(GetData(ViewModeControl), ImVec2(30 + ViewModeTextSize.x, 32)))
    {
        ImGui::OpenPopup("ViewModeControl");
    }

    if (ImGui::BeginPopup("ViewModeControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewModeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewMode() == i);
            if (ImGui::Selectable(ViewModeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewMode((EViewModeIndex)i);
                FEngineLoop::GraphicDevice.ChangeRasterizer(ActiveViewport->GetViewMode());
                FEngineLoop::Renderer.ChangeViewMode(ActiveViewport->GetViewMode());
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowControl");
    }

    const char* items[] = { "AABB", "Primitive", "BillBoard", "UUID", "Fog", "LightHeatMap", "Icons", "LightVisualize"};
    uint64 ActiveViewportFlags = ActiveViewport->GetShowFlag();

    if (ImGui::BeginPopup("ShowControl"))
    {
        bool selected[IM_ARRAYSIZE(items)] =
        {
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_UUIDText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Fog)) !=0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_LightHeatMap)) !=0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Icons)) !=0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_LightVisualize)) !=0,
        };  // 각 항목의 체크 상태 저장

        for (int i = 0; i < IM_ARRAYSIZE(items); i++)
        {
            ImGui::Checkbox(items[i], &selected[i]);
        }
        ActiveViewport->SetShowFlag(ConvertSelectionToFlags(selected));
        ImGui::EndPopup();
    }
}

// code is so dirty / Please refactor
void ViewportControlEditorPanel::CreateSRTButton(ImVec2 IconSize, ImFont* IconFont) const
{
    ImGui::PushFont(IconFont);
    float ContentWidth = ImGui::GetWindowContentRegionMax().x;

    ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));
    
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    AEditorPlayer* Player = Engine->GetEditorPlayer();

    ImVec4 ActiveColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

    ControlMode ControlMode = Player->GetControlMode();

    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9bc", IconSize)) // Move
    {
        Player->SetMode(CM_TRANSLATION);
    }
    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (ControlMode == CM_ROTATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9d3", IconSize)) // Rotate
    {
        Player->SetMode(CM_ROTATION);
    }
    if (ControlMode == CM_ROTATION)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (ControlMode == CM_SCALE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9ab", IconSize)) // Scale
    {
        Player->SetMode(CM_SCALE);
    }
    if (ControlMode == CM_SCALE)
    {
        ImGui::PopStyleColor();
    }

    ImGui::PopFont();
}


uint64 ViewportControlEditorPanel::ConvertSelectionToFlags(const bool selected[]) const
{
    uint64 flags = EEngineShowFlags::None;

    if (selected[0])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
    if (selected[1])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
    if (selected[2])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_BillboardText);
    if (selected[3])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_UUIDText);
    if (selected[4])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Fog);
    if (selected[5])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_LightHeatMap);
    if (selected[6])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Icons);
    if (selected[7])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_LightVisualize);
    return flags;
}

void ViewportControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Rect.Left = clientRect.left;
    Rect.Right = clientRect.right;
    Rect.Top = clientRect.top;
    Rect.Bottom = clientRect.bottom;
}
