#include "OutlinerEditorPanel.h"
#include "World/World.h"
#include "GameFramework/Actor.h"
#include "Engine/EditorEngine.h"
#include <functional>

void OutlinerEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();

    Padding = FSlateRect(5, 5, 1, 0);
    AnchorMin = FVector2D(0.8f, 0);
    AnchorMax = FVector2D(1.f, 0.3f);
    
    FSlateRect PanelRect = FSlateRect(0 ,0, 0, 0);
    PanelRect.Left = Rect.GetWidth() * AnchorMin.X + Padding.Left;
    PanelRect.Top = Rect.GetHeight() * AnchorMin.Y + Padding.Top;

    PanelRect.Right = Rect.GetWidth() * AnchorMax.X - Padding.Right;
    PanelRect.Bottom = Rect.GetHeight() * AnchorMax.Y - Padding.Bottom;

    ImVec2 MinSize(140, 100);
    ImVec2 MaxSize(FLT_MAX, 500);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelRect.Left, PanelRect.Top), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelRect.GetWidth(), PanelRect.GetHeight()), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    /* Render Start */
    ImGui::Begin("Outliner", nullptr, PanelFlags);

    
    ImGui::BeginChild("Objects");
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        ImGui::EndChild();
        ImGui::End();
        
        return;
    }

    std::function<void(USceneComponent*)> CreateNode = [&CreateNode, &Engine](USceneComponent* InComp)->void
    {
        FString Name = InComp->GetName();

        ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None;
        if (InComp->GetAttachChildren().Num() == 0)
            Flags |= ImGuiTreeNodeFlags_Leaf;

        bool NodeOpen = ImGui::TreeNodeEx(*Name, Flags);

        if (ImGui::IsItemClicked())
        {
            Engine->SelectActor(InComp->GetOwner());
            Engine->SelectComponent(InComp);
        }

        if (NodeOpen)
        {
            for (USceneComponent* Child : InComp->GetAttachChildren())
            {
                CreateNode(Child);
            }
            ImGui::TreePop(); // 트리 닫기
        }
    };

    for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors)
    {
        ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None;

        bool NodeOpen = ImGui::TreeNodeEx(*Actor->GetName(), Flags);

        if (ImGui::IsItemClicked())
        {
            Engine->SelectActor(Actor);
            Engine->DeselectComponent(Engine->GetSelectedComponent());
        }

        if (NodeOpen)
        {
            CreateNode(Actor->GetRootComponent());
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();

    ImGui::End();
}
    
void OutlinerEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Rect.Left = clientRect.left;
    Rect.Right = clientRect.right;
    Rect.Top = clientRect.top;
    Rect.Bottom = clientRect.bottom;
}
