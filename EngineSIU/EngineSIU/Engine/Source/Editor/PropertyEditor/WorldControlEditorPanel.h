#pragma once
#include "Define.h"
#include "ImGUI/imgui.h"

#include "UnrealEd/EditorPanel.h"

class WorldControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateAddPrimitive(ImVec2 ButtonSize, ImFont* IconFont) const;
    void CreatePIEButton(ImVec2 ButtonSize, ImFont* IconFont) const;
    
private:
    float Width = 300, Height = 100;

    bool bOpenMenu = false;
};
