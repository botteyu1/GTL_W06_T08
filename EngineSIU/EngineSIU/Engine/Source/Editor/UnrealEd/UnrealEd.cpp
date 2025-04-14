#include "UnrealEd.h"
#include "EditorPanel.h"

#include "PropertyEditor/ViewportControlEditorPanel.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"
#include "PropertyEditor/WorldControlEditorPanel.h"

void UnrealEd::Initialize(HWND hWnd)
{
    auto WorldControlPanel = std::make_shared<WorldControlEditorPanel>();
    AddEditorPanel("WorldControlPanel", WorldControlPanel);

    // On Add Viewport ->
    // On Remove Viewport ->
    // On Resize Viewport ->

    // if ViewportIsValid -> Render
    auto ViewportControlPanel = std::make_shared<ViewportControlEditorPanel>();
    AddEditorPanel("ViewportControlPanel, {ViewportNum}", ViewportControlPanel);
    
    auto OutlinerPanel = std::make_shared<OutlinerEditorPanel>();
    AddEditorPanel("OutlinerPanel", OutlinerPanel);
    
    auto PropertyPanel = std::make_shared<PropertyEditorPanel>();
    AddEditorPanel("PropertyPanel", PropertyPanel);

    OnResize(hWnd);
}

void UnrealEd::Render() const
{
    for (const auto& Panel : Panels)
    {
        Panel.Value->Render();
    }
}

void UnrealEd::AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel)
{
    Panels[PanelId] = EditorPanel;
}

void UnrealEd::RemoveEditorPanel(const FString& PanelId)
{
    if (Panels.Contains(PanelId))
    {
        Panels.Remove(PanelId);
    }
}

void UnrealEd::OnResize(HWND hWnd) const
{
    for (auto& Panel : Panels)
    {
        Panel.Value->OnResize(hWnd);
    }
}

std::shared_ptr<UEditorPanel> UnrealEd::GetEditorPanel(const FString& PanelId)
{
    return Panels[PanelId];
}
