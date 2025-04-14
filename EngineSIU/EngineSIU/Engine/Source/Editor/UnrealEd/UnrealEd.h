#pragma once
#include "Container/Map.h"
#include "Container/String.h"

class UEditorPanel;

class UnrealEd
{
public:
    UnrealEd() = default;
    ~UnrealEd() = default;
    void Initialize(HWND hWnd);
    
     void Render() const;
     void OnResize(HWND hWnd) const;
    
    void AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel);
    void RemoveEditorPanel(const FString& PanelId);
    std::shared_ptr<UEditorPanel> GetEditorPanel(const FString& PanelId);

private:
    TMap<FString, std::shared_ptr<UEditorPanel>> Panels;
    //TMap<ViewportNameIndexName, std::shared_ptr<UEditorPanel>> PanelsPerViewportClient;
};
